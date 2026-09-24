"""Static checker for Call of Duty: World at War (T4) singleplayer GSC mods.

The T4 script compiler runs when a level loads, and any compile error (bad syntax, an unknown
builtin, a missing script) stops the level from loading. Testing that on a console or in Xenia
is slow, so this checker catches the same class of errors offline:

- syntax the stock compiler rejects (including unary minus on variables, which T4 lacks)
- calls to functions/methods that are neither builtins nor defined in the file or its includes
- builtins called as methods (or the other way round) and threads started on builtins
- references to scripts that are not guaranteed to exist on every singleplayer map
  (e.g. maps\\_zombiemode_* scripts, which campaign levels do not contain)
- duplicate functions, collisions with included scripts, too many call arguments
- locals read where they are not assigned on every path (the compiler's "uninitialised variable"
  error), plus break/continue outside loops and assignments to self/level

Builtin names come from t4_sp_index.json (see build_index.py); CoD Xe's own GSC builtins are read
from src/game/t4/sp/components/gsc.cpp so the list stays in sync with the plugin.

Usage: python tools/gsc_check/gsc_check.py resources/t4/_codxe/mods/mod_menu
"""
import argparse
import json
import os
import re
import sys

# ---------------------------------------------------------------------------
# Lexer
# ---------------------------------------------------------------------------

KEYWORDS = {
    "if", "else", "while", "for", "switch", "case", "default", "break", "continue",
    "return", "wait", "waittillframeend", "thread", "true", "false", "undefined",
    "breakpoint",
}

TOKEN_RE = re.compile(
    r"""
    (?P<ws>[ \t\r\n]+)
  | (?P<linecomment>//[^\n]*)
  | (?P<blockcomment>/\*.*?\*/)
  | (?P<devopen>/\#)
  | (?P<devclose>\#/)
  | (?P<directive>\#[A-Za-z_]+)
  | (?P<string>"(?:\\.|[^"\\\n])*")
  | (?P<number>(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?)
  | (?P<ident>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<op>::|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||\+=|-=|\*=|/=|%=|&=|\|=|\^=|[-+*/%<>=!~&|^?:;,.(){}\[\]\\])
    """,
    re.VERBOSE | re.DOTALL,
)


class Token:
    __slots__ = ("kind", "value", "line")

    def __init__(self, kind, value, line):
        self.kind = kind
        self.value = value
        self.line = line

    def __repr__(self):
        return "%s(%r)@%d" % (self.kind, self.value, self.line)


class LexError(Exception):
    pass


def tokenize(src, strip_dev=True):
    """Return a list of tokens. Dev blocks (/# ... #/) are dropped when strip_dev."""
    tokens = []
    pos = 0
    line = 1
    dev_depth = 0
    n = len(src)
    while pos < n:
        m = TOKEN_RE.match(src, pos)
        if not m:
            raise LexError("line %d: unexpected character %r" % (line, src[pos]))
        kind = m.lastgroup
        value = m.group(kind)
        if kind == "devopen":
            dev_depth += 1
        elif kind == "devclose":
            dev_depth = max(0, dev_depth - 1)
        elif kind not in ("ws", "linecomment", "blockcomment"):
            if not (strip_dev and dev_depth):
                if kind == "ident" and value.lower() in KEYWORDS:
                    kind = "kw"
                    value = value.lower()
                tokens.append(Token(kind, value, line))
        line += value.count("\n")
        pos = m.end()
    tokens.append(Token("eof", "", line))
    return tokens

HERE = os.path.dirname(os.path.abspath(__file__))

# Stock scripts _load.gsc pulls into every singleplayer level (campaign and zombies).
SAFE_SCRIPTS = [
    "common_scripts\\utility",
    "maps\\_utility",
    "maps\\_utility_code",
    "maps\\_hud_util",
    "maps\\_laststand",
    "maps\\_collectibles",
    "maps\\_collectibles_game",
    "animscripts\\death",
]

# Builtins that stock scripts call rarely but that are confirmed to exist on every SP level,
# because the stock script calling them is itself compiled on every level (or a launch map).
VERIFIED_RARE = {
    "setcollectible",  # maps\\_collectibles.gsc
    "unsetcollectible",  # maps\\_collectibles_game.gsc
    "setplayerignoreradiusdamage",  # maps\\ber2_event1.gsc
    "iprintlnbold",  # method form in maps\\_hud_message.gsc
}

ENTITY_KEYWORDS = {"self", "level", "game", "anim"}

# The compiler keeps locals in a fixed-size frame; stay well clear of the limit.
MAX_LOCALS = 48
BINARY_PRECEDENCE = [
    ("||",),
    ("&&",),
    ("|",),
    ("^",),
    ("&",),
    ("==", "!="),
    ("<", ">", "<=", ">="),
    ("<<", ">>"),
    ("+", "-"),
    ("*", "/", "%"),
]
ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}


class ParseError(Exception):
    def __init__(self, line, msg):
        Exception.__init__(self, "line %d: %s" % (line, msg))
        self.line = line


class Call:
    def __init__(self, kind, name, path, nargs, line, threaded):
        self.kind = kind  # "func" (no object) or "method" (object call)
        self.name = name.lower() if name else None
        self.path = path.lower() if path else None
        self.nargs = nargs
        self.line = line
        self.threaded = threaded


class Function:
    def __init__(self, name, params, line):
        self.name = name
        self.params = params
        self.line = line
        self.calls = []
        self.pointers = []  # (path, name, line)
        self.uninit = []  # (name, line)
        self.locals = set(params)


class Postfix:
    """What parse_postfix saw: the local it starts from (if any) and the operations after it."""

    def __init__(self, root_local, entity_root, chain, is_call, line):
        self.root_local = root_local
        self.entity_root = entity_root
        self.chain = chain
        self.is_call = is_call
        self.line = line


class Parser:
    """Recursive descent parser for T4 GSC.

    It also mirrors the compiler's "uninitialised variable" check, which is a *compile* error in
    T4: a local may only be read where every path to the read has assigned it. Assignments inside
    an if without else, inside a loop body or inside a switch case do not count after it. Paths
    that end in return/break/continue are left out of the merge.
    """

    def __init__(self, tokens):
        self.t = tokens
        self.i = 0
        self.includes = []
        self.functions = []
        self.fn = None
        self.defined = set()  # locals definitely assigned on the current path
        self.live = True  # False after return/break/continue until paths merge again
        self.ctx = []  # enclosing loops/switches: {"kind": "loop"|"switch", "breaks": [set, ...]}

    # token helpers -------------------------------------------------------
    def peek(self, k=0):
        return self.t[min(self.i + k, len(self.t) - 1)]

    def at(self, value, k=0):
        if value in ("[[", "]]"):
            return self.at(value[0], k) and self.at(value[1], k + 1)
        tok = self.peek(k)
        return tok.kind in ("op", "kw") and tok.value == value

    def next(self):
        tok = self.t[self.i]
        self.i += 1
        return tok

    def expect(self, value):
        if value in ("[[", "]]"):
            self.expect(value[0])
            return self.expect(value[1])
        tok = self.next()
        if tok.value != value or tok.kind not in ("op", "kw"):
            raise ParseError(tok.line, "expected %r, got %r" % (value, tok.value))
        return tok

    def expect_ident(self):
        tok = self.next()
        if tok.kind != "ident":
            raise ParseError(tok.line, "expected identifier, got %r" % tok.value)
        return tok

    # definite assignment -------------------------------------------------
    def use_local(self, name, line):
        if name not in self.defined:
            self.fn.uninit.append((name, line))

    def assign_local(self, name):
        self.defined.add(name)
        self.fn.locals.add(name)

    def terminate(self):
        self.live = False

    def merge(self, paths, fallback, was_live):
        """Continue after a branch point. paths are the `defined` sets of every path that
        reaches this point."""
        if paths and was_live:
            merged = set(paths[0])
            for p in paths[1:]:
                merged &= p
            self.defined = merged
            self.live = True
        else:
            self.defined = set(fallback)
            self.live = False

    def const_true_condition(self):
        """True when the loop condition is a literal `1`/`true` followed by `)`."""
        tok = self.peek()
        if not self.at(")", 1):
            return False
        if tok.kind == "kw" and tok.value == "true":
            return True
        return tok.kind == "number" and float(tok.value) != 0

    # top level -----------------------------------------------------------
    def parse_file(self):
        while self.peek().kind != "eof":
            tok = self.peek()
            if tok.kind == "directive":
                self.next()
                if tok.value.lower() == "#include":
                    parts = []
                    while not self.at(";"):
                        p = self.next()
                        if p.kind == "eof":
                            raise ParseError(tok.line, "unterminated #include")
                        parts.append(p.value)
                    self.expect(";")
                    self.includes.append(("".join(parts).lower(), tok.line))
                elif tok.value.lower() == "#using_animtree":
                    raise ParseError(tok.line, "#using_animtree is not allowed in this mod")
                else:
                    raise ParseError(tok.line, "unknown directive %s" % tok.value)
            elif tok.kind == "ident":
                self.parse_function()
            else:
                raise ParseError(tok.line, "unexpected %r at file scope" % tok.value)

    def parse_function(self):
        name = self.expect_ident()
        self.expect("(")
        params = []
        if not self.at(")"):
            while True:
                params.append(self.expect_ident().value.lower())
                if self.at(","):
                    self.next()
                    continue
                break
        self.expect(")")
        fn = Function(name.value.lower(), params, name.line)
        self.fn = fn
        self.defined = set(params)
        self.live = True
        self.ctx = []
        self.parse_block()
        self.functions.append(fn)
        self.fn = None

    # statements ----------------------------------------------------------
    def parse_block(self):
        self.expect("{")
        while not self.at("}"):
            if self.peek().kind == "eof":
                raise ParseError(self.peek().line, "unexpected end of file (missing '}')")
            self.parse_statement()
        self.expect("}")

    def parse_loop_body(self, pre, was_live, infinite):
        self.defined, self.live = set(pre), was_live
        self.ctx.append({"kind": "loop", "breaks": []})
        self.parse_statement()
        ctx = self.ctx.pop()
        if infinite:
            # the only way out of `for(;;)` / `while(1)` is a break
            self.merge(ctx["breaks"], pre, was_live)
        else:
            self.defined, self.live = set(pre), was_live

    def parse_statement(self):
        tok = self.peek()
        if tok.kind == "op" and tok.value == "{":
            self.parse_block()
            return
        if tok.kind == "op" and tok.value == ";":
            raise ParseError(tok.line, "empty statement")
        if tok.kind == "kw":
            v = tok.value
            if v == "if":
                self.next()
                self.expect("(")
                self.parse_expr()
                self.expect(")")
                pre, was_live = set(self.defined), self.live
                self.parse_statement()
                paths = [self.defined] if self.live else []
                self.defined, self.live = set(pre), was_live
                if self.at("else"):
                    self.next()
                    self.parse_statement()
                    if self.live:
                        paths.append(self.defined)
                else:
                    paths.append(pre)
                self.merge(paths, pre, was_live)
                return
            if v == "while":
                self.next()
                self.expect("(")
                infinite = self.const_true_condition()
                self.parse_expr()
                self.expect(")")
                self.parse_loop_body(set(self.defined), self.live, infinite)
                return
            if v == "for":
                self.next()
                self.expect("(")
                if not self.at(";"):
                    self.parse_simple_statement()
                self.expect(";")
                infinite = self.at(";")
                if not infinite:
                    infinite = self.const_true_condition()
                    self.parse_expr()
                self.expect(";")
                pre, was_live = set(self.defined), self.live
                if not self.at(")"):
                    self.parse_simple_statement()  # the increment; its assignments do not escape
                self.expect(")")
                self.parse_loop_body(pre, was_live, infinite)
                return
            if v == "switch":
                self.next()
                self.expect("(")
                self.parse_expr()
                self.expect(")")
                self.expect("{")
                pre, was_live = set(self.defined), self.live
                self.ctx.append({"kind": "switch", "breaks": []})
                seen_label = has_default = False
                case_values = set()
                self.live = False
                while not self.at("}"):
                    if self.at("case") or self.at("default"):
                        if self.at("case"):
                            self.next()
                            lit = self.next()
                            negative = ""
                            if lit.value == "-" and self.peek().kind == "number":
                                lit = self.next()
                                negative = "-"
                            if lit.kind not in ("string", "number"):
                                raise ParseError(lit.line, "case label must be a string or number literal")
                            value = (lit.kind, negative + lit.value.lower())
                            if value in case_values:
                                raise ParseError(lit.line, "duplicate case %s" % lit.value)
                            case_values.add(value)
                        else:
                            if has_default:
                                raise ParseError(self.peek().line, "switch has more than one default")
                            self.next()
                            has_default = True
                        self.expect(":")
                        seen_label = True
                        # reachable by the jump from the switch (and maybe by fall-through)
                        self.defined, self.live = set(pre), was_live
                    else:
                        if not seen_label:
                            raise ParseError(self.peek().line, "statement before first case label")
                        self.parse_statement()
                self.expect("}")
                ctx = self.ctx.pop()
                paths = list(ctx["breaks"])
                if self.live:
                    paths.append(self.defined)
                if not has_default:
                    paths.append(pre)
                self.merge(paths, pre, was_live)
                return
            if v == "break":
                self.next()
                self.expect(";")
                if not self.ctx:
                    raise ParseError(tok.line, "'break' outside of a loop or switch")
                if self.live:
                    self.ctx[-1]["breaks"].append(set(self.defined))
                self.terminate()
                return
            if v == "continue":
                self.next()
                self.expect(";")
                if not any(c["kind"] == "loop" for c in self.ctx):
                    raise ParseError(tok.line, "'continue' outside of a loop")
                self.terminate()
                return
            if v in ("waittillframeend", "breakpoint"):
                self.next()
                self.expect(";")
                return
            if v == "return":
                self.next()
                if not self.at(";"):
                    self.parse_expr()
                self.expect(";")
                self.terminate()
                return
            if v == "wait":
                self.next()
                self.parse_expr()
                self.expect(";")
                return
            if v == "else":
                raise ParseError(tok.line, "'else' without 'if'")
            if v in ("case", "default"):
                raise ParseError(tok.line, "'%s' outside of switch" % v)
        self.parse_simple_statement()
        self.expect(";")

    def parse_simple_statement(self):
        """Assignment, increment/decrement or call. Anything else is a compile error."""
        start = self.peek()
        pf = self.parse_postfix(statement=True)
        if pf.is_call:
            return
        root = pf.root_local
        # a local, or anything ending in .field / [index] (including getEnt(...).field and (x).field)
        assignable = (root is not None and not pf.chain) or (pf.chain and pf.chain[-1] in (".", "["))

        if self.at("++") or self.at("--"):
            if not assignable:
                raise ParseError(start.line, "cannot increment/decrement this expression")
            self.next()
            if root:
                self.use_local(root, pf.line)
            return

        tok = self.peek()
        if tok.kind == "op" and tok.value in ASSIGN_OPS:
            if not assignable:
                if pf.entity_root and not pf.chain:
                    raise ParseError(tok.line, "cannot assign to '%s'" % start.value)
                raise ParseError(tok.line, "left side of assignment is not assignable")
            self.next()
            if root and not pf.chain:
                if tok.value != "=":
                    self.use_local(root, pf.line)
                self.parse_expr()
                self.assign_local(root)
            elif root and tok.value == "=" and all(op == "[" for op in pf.chain):
                # `arr[i] = x` / `arr[i][j] = x` create arr when it does not exist yet
                self.parse_expr()
                self.assign_local(root)
            else:
                if root:
                    self.use_local(root, pf.line)
                self.parse_expr()
            return
        raise ParseError(start.line, "statement has no effect (expected call or assignment)")

    # expressions ---------------------------------------------------------
    def parse_expr(self, level=0):
        if level == len(BINARY_PRECEDENCE):
            return self.parse_unary()
        self.parse_expr(level + 1)
        while True:
            tok = self.peek()
            if tok.kind == "op" and tok.value in BINARY_PRECEDENCE[level]:
                self.next()
                self.parse_expr(level + 1)
            else:
                break

    def parse_unary(self):
        tok = self.peek()
        if tok.kind == "op" and tok.value in ("!", "~"):
            self.next()
            self.parse_unary()
            return
        if tok.kind == "op" and tok.value == "-":
            self.next()
            if self.peek().kind != "number":
                raise ParseError(tok.line, "unary minus is only allowed on number literals (use 'x * -1')")
            self.next()
            return
        self.parse_postfix()

    def parse_call_args(self):
        self.expect("(")
        n = 0
        if not self.at(")"):
            while True:
                self.parse_expr()
                n += 1
                if self.at(","):
                    self.next()
                    continue
                break
        self.expect(")")
        return n

    def parse_waittill_args(self):
        """`waittill(<notify>, a, b)` assigns the extra identifiers from the notify payload."""
        self.expect("(")
        self.parse_expr()
        n = 1
        while self.at(","):
            self.next()
            self.assign_local(self.expect_ident().value.lower())
            n += 1
        self.expect(")")
        return n

    def parse_callee_call(self, has_object, threaded):
        """Parse `name(...)`, `path::name(...)` or `[[expr]](...)` at the cursor."""
        tok = self.peek()
        if self.at("[["):
            self.expect("[[")
            self.parse_expr()
            self.expect("]]")
            self.parse_call_args()
            return
        path = self.try_parse_path_prefix()
        name = self.expect_ident()
        if has_object and not path and name.value.lower() == "waittill":
            nargs = self.parse_waittill_args()
        else:
            nargs = self.parse_call_args()
        self.fn.calls.append(Call("method" if has_object else "func", name.value, path, nargs, tok.line, threaded))

    def try_parse_path_prefix(self):
        """If the cursor is on `a\\b\\c::`, consume it and return the path."""
        j = 0
        parts = []
        while True:
            tok = self.peek(j)
            if tok.kind != "ident":
                return None
            parts.append(tok.value)
            nxt = self.peek(j + 1)
            if nxt.kind == "op" and nxt.value == "\\":
                parts.append("\\")
                j += 2
                continue
            if nxt.kind == "op" and nxt.value == "::":
                for _ in range(j + 2):
                    self.next()
                return "".join(parts)
            return None

    def path_pointer_ahead(self):
        j = 0
        while True:
            if self.peek(j).kind != "ident":
                return False
            nxt = self.peek(j + 1)
            if nxt.kind == "op" and nxt.value == "\\":
                j += 2
                continue
            return nxt.kind == "op" and nxt.value == "::"

    def looks_like_call_start(self):
        tok = self.peek()
        if self.at("[["):
            return True
        if tok.kind != "ident":
            return False
        if self.path_pointer_ahead():
            return True
        return self.peek(1).kind == "op" and self.peek(1).value == "("

    def parse_postfix(self, statement=False):
        tok = self.peek()
        root_local = None
        entity_root = False
        is_call = False
        chain = []

        if tok.kind == "kw" and tok.value == "thread":
            self.next()
            self.parse_callee_call(False, True)
            return Postfix(None, False, ["call"], True, tok.line)
        if tok.kind == "op" and tok.value == "::":
            self.next()
            name = self.expect_ident()
            self.fn.pointers.append((None, name.value.lower(), name.line))
        elif tok.kind == "ident" and self.path_pointer_ahead():
            path = self.try_parse_path_prefix()
            name = self.expect_ident()
            if self.at("("):
                nargs = self.parse_call_args()
                self.fn.calls.append(Call("func", name.value, path, nargs, name.line, False))
                is_call = True
            else:
                self.fn.pointers.append((path.lower(), name.value.lower(), name.line))
        elif tok.kind == "ident" and self.at("(", 1):
            self.parse_callee_call(False, False)
            is_call = True
        elif self.at("[["):
            self.parse_callee_call(False, False)
            is_call = True
        elif tok.kind == "ident":
            self.next()
            if tok.value.lower() in ENTITY_KEYWORDS:
                entity_root = True
            else:
                root_local = tok.value.lower()
        elif tok.kind == "kw" and tok.value in ("true", "false", "undefined"):
            self.next()
        elif tok.kind in ("number", "string"):
            self.next()
        elif tok.kind == "op" and tok.value == "&":
            self.next()
            s = self.next()
            if s.kind != "string":
                raise ParseError(s.line, "expected string after '&'")
            raise ParseError(s.line, "localized strings (&\"...\") need a localize entry; use plain strings")
        elif tok.kind == "op" and tok.value == "%":
            raise ParseError(tok.line, "animation references (%anim) are map specific; do not use them")
        elif tok.kind == "op" and tok.value == "[":
            self.next()
            self.expect("]")
        elif tok.kind == "op" and tok.value == "(":
            self.next()
            self.parse_expr()
            if self.at(","):
                self.next()
                self.parse_expr()
                self.expect(",")
                self.parse_expr()
            self.expect(")")
        else:
            raise ParseError(tok.line, "unexpected %r in expression" % tok.value)

        while True:
            nxt = self.peek()
            if nxt.kind == "op" and nxt.value == ".":
                self.next()
                self.expect_ident()
                chain.append(".")
                is_call = False
            elif nxt.kind == "op" and nxt.value == "[" and not self.at("[["):
                self.next()
                self.parse_expr()
                self.expect("]")
                chain.append("[")
                is_call = False
            elif (nxt.kind == "kw" and nxt.value == "thread") or self.looks_like_call_start():
                threaded = False
                if nxt.kind == "kw" and nxt.value == "thread":
                    self.next()
                    threaded = True
                self.parse_callee_call(True, threaded)
                chain.append("call")
                is_call = True
            else:
                break

        # Reads of a local are checked here; a statement's assignment target is checked by
        # parse_simple_statement once it knows whether the local is being written.
        if root_local and (not statement or "call" in chain):
            self.use_local(root_local, tok.line)
        return Postfix(root_local, entity_root, chain, is_call, tok.line)


# ---------------------------------------------------------------------------


def load_codxe_builtins(cpp_path):
    """Read the gsc::Entry tables codxe registers for T4 SP."""
    funcs, methods = set(), set()
    if not cpp_path or not os.path.exists(cpp_path):
        return funcs, methods
    with open(cpp_path, encoding="utf-8") as f:
        src = f.read()
    for m in re.finditer(r"gsc::Entry<(BuiltinMethod|BuiltinFunction)>\s+\w+\[\]\s*=\s*\{(.*?)\};", src, re.S):
        names = re.findall(r'\{\s*"([a-z0-9_]+)"', m.group(2))
        (methods if m.group(1) == "BuiltinMethod" else funcs).update(names)
    return funcs, methods


def load_mod(mod_dir):
    files = {}
    for dirpath, _, names in os.walk(mod_dir):
        for name in names:
            if name.lower().endswith(".gsc"):
                full = os.path.join(dirpath, name)
                rel = os.path.relpath(full, mod_dir).replace("/", "\\")[:-4].lower()
                files[rel] = full
    return files


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mod_dir")
    ap.add_argument("--index", default=os.path.join(HERE, "t4_sp_index.json"))
    ap.add_argument("--safe-scripts", default=",".join(SAFE_SCRIPTS),
                    help="comma separated stock scripts that every singleplayer level loads")
    ap.add_argument("--rare", type=int, default=2, help="warn when a builtin appears in fewer stock files than this")
    ap.add_argument("--codxe-gsc", default=os.path.join(HERE, "..", "..", "src", "game", "t4", "sp", "components", "gsc.cpp"),
                    help="CoD Xe source file that registers extra T4 SP builtins")
    args = ap.parse_args()

    with open(args.index) as f:
        index = json.load(f)
    builtin_f = dict(index["functions"])
    builtin_m = dict(index["methods"])
    cx_f, cx_m = load_codxe_builtins(args.codxe_gsc)
    for name in cx_f:
        builtin_f[name] = 99
    for name in cx_m:
        builtin_m[name] = 99
    stock = index["scripts"]
    safe = set(s.strip().lower() for s in args.safe_scripts.split(",") if s.strip())

    mod_files = load_mod(args.mod_dir)
    parsed = {}
    errors = []
    warnings = []

    for key, path in sorted(mod_files.items()):
        with open(path, encoding="latin-1") as f:
            src = f.read()
        if "/#" in src or "#/" in src:
            warnings.append("%s: dev blocks are stripped in retail builds" % key)
        try:
            p = Parser(tokenize(src, strip_dev=True))
            p.parse_file()
        except (ParseError, LexError) as e:
            errors.append("%s: syntax: %s" % (key, e))
            continue
        parsed[key] = p

    def script_defs(script):
        if script in parsed:
            return {fn.name: len(fn.params) for fn in parsed[script].functions}
        if script in stock:
            return stock[script]["defs"]
        return None

    def script_exists(script):
        return script in parsed or script in stock

    used_builtins = {}
    for key, p in sorted(parsed.items()):
        local = {}
        for fn in p.functions:
            if fn.name in local:
                errors.append("%s:%d: function '%s' defined twice" % (key, fn.line, fn.name))
            local[fn.name] = len(fn.params)
            if fn.name in builtin_f or fn.name in builtin_m:
                warnings.append("%s:%d: function '%s' shadows a builtin" % (key, fn.line, fn.name))

        included = {}
        for inc, line in p.includes:
            if inc not in parsed and inc not in safe:
                errors.append("%s:%d: #include %s is not a mod script or a script guaranteed in every SP map" % (key, line, inc))
            defs = script_defs(inc)
            if defs is None:
                errors.append("%s:%d: #include %s: script not found" % (key, line, inc))
                continue
            for name, n in defs.items():
                if name in local:
                    errors.append("%s: function '%s' collides with one in included %s" % (key, name, inc))
                included.setdefault(name, (inc, n))

        def resolve_script_fn(name):
            if name in local:
                return local[name]
            if name in included:
                return included[name][1]
            return None

        for fn in p.functions:
            for c in fn.calls:
                where = "%s:%d" % (key, c.line)
                if c.path:
                    if c.path not in parsed and c.path not in safe:
                        errors.append("%s: call into %s which is not guaranteed to exist on every SP map" % (where, c.path))
                    defs = script_defs(c.path)
                    if defs is None:
                        errors.append("%s: script %s not found" % (where, c.path))
                    elif c.name not in defs:
                        errors.append("%s: %s::%s is not defined" % (where, c.path, c.name))
                    elif c.nargs > defs[c.name]:
                        errors.append("%s: %s::%s takes %d args, called with %d" % (where, c.path, c.name, defs[c.name], c.nargs))
                    continue
                n = resolve_script_fn(c.name)
                if n is not None:
                    if c.nargs > n:
                        errors.append("%s: %s takes %d args, called with %d" % (where, c.name, n, c.nargs))
                    continue
                if c.threaded:
                    errors.append("%s: cannot thread unknown/builtin function '%s'" % (where, c.name))
                    continue
                table = builtin_m if c.kind == "method" else builtin_f
                if c.name in table:
                    used_builtins[(c.kind, c.name)] = table[c.name]
                    continue
                other = builtin_f if c.kind == "method" else builtin_m
                if c.name in other:
                    errors.append("%s: '%s' is a builtin %s, called as a %s" % (
                        where, c.name, "function" if c.kind == "method" else "method", c.kind))
                else:
                    errors.append("%s: unknown %s '%s'" % (where, c.kind, c.name))
            for path, name, line in fn.pointers:
                where = "%s:%d" % (key, line)
                if path:
                    defs = script_defs(path)
                    if defs is None or name not in defs:
                        errors.append("%s: pointer %s::%s is not defined" % (where, path, name))
                elif resolve_script_fn(name) is None:
                    errors.append("%s: pointer ::%s is not defined in file or includes" % (where, name))
            seen = set()
            for name, line in fn.uninit:
                if (name, line) in seen:
                    continue
                seen.add((name, line))
                errors.append("%s:%d: uninitialised variable '%s' in %s() - not assigned on every path "
                              "before this read (the T4 compiler rejects this)" % (key, line, name, fn.name))
            if len(fn.locals) > MAX_LOCALS:
                warnings.append("%s:%d: %s() uses %d locals; keep it under %d" % (
                    key, fn.line, fn.name, len(fn.locals), MAX_LOCALS))

    for (kind, name), count in sorted(used_builtins.items()):
        if count < args.rare and name not in VERIFIED_RARE:
            warnings.append("rare builtin %s '%s' (used in %d stock file(s)) - verify it exists on 360 TU7" % (kind, name, count))

    for w in warnings:
        print("warning:", w)
    for e in errors:
        print("error:", e)
    print("%d file(s), %d function(s), %d builtin(s) used, %d error(s), %d warning(s)" % (
        len(mod_files), sum(len(p.functions) for p in parsed.values()), len(used_builtins), len(errors), len(warnings)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
