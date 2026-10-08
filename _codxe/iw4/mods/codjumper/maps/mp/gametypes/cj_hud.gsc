GetSpeedXY()
{
    velocity = self getVelocity();
    horizontalSpeed = int(sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]));
    return horizontalSpeed;
}

CJ_HUD_ALPHA = 0.35; // Default alpha for CJ HUD elements

StartHUD()
{
    self endon("disconnect");
    self endon("death");
    self endon("end_respawn");
    self endon("joined_spectators");

    // IMPORTANT: Destroy old HUD elements if they exist
    if (isdefined(self.cj.speed_hudelem))
        self.cj.speed_hudelem destroy();
    if (isdefined(self.cj.z_origin_hudelem))
        self.cj.z_origin_hudelem destroy();

    speed_hudelem = self maps\mp\gametypes\_hud_util::createFontString("small", 1.2);
    speed_hudelem.alignX = "left";
    speed_hudelem.alignY = "top";
    speed_hudelem.horzAlign = "fullscreen";
    speed_hudelem.vertAlign = "fullscreen";
    speed_hudelem.alpha = CJ_HUD_ALPHA;
    speed_hudelem.x = 0;
    speed_hudelem.y = 453;
    speed_hudelem.color = (1, 1, 1);
    self.cj.speed_hudelem = speed_hudelem;

    z_origin_hudelem = self maps\mp\gametypes\_hud_util::createFontString("small", 1.2);
    z_origin_hudelem.alignX = "left";
    z_origin_hudelem.alignY = "top";
    z_origin_hudelem.horzAlign = "fullscreen";
    z_origin_hudelem.vertAlign = "fullscreen";
    z_origin_hudelem.alpha = CJ_HUD_ALPHA;
    z_origin_hudelem.x = 0;
    z_origin_hudelem.y = 465;
    z_origin_hudelem.color = (1, 1, 1);
    self.cj.z_origin_hudelem = z_origin_hudelem;

    for (;;)
    {
        speed_hudelem SetValue(self GetSpeedXY());
        z_origin_hudelem SetValue(self.origin[2]);
        wait 0.05;
    }
}

ToggleHUD()
{
    if (!isdefined(self.cj.speed_hudelem) || !isdefined(self.cj.z_origin_hudelem))
        return;

    if (self.cj.speed_hudelem.alpha > 0)
    {
        self.cj.speed_hudelem.alpha = 0;
        self.cj.z_origin_hudelem.alpha = 0;
        self iPrintln("Speedometer [^1OFF^7]");
    }
    else
    {
        self.cj.speed_hudelem.alpha = CJ_HUD_ALPHA;
        self.cj.z_origin_hudelem.alpha = CJ_HUD_ALPHA;
        self iPrintln("Speedometer [^2ON^7]");
    }
}
