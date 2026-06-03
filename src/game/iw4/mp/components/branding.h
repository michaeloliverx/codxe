#pragma once

namespace iw4
{

namespace mp
{
class Branding : public Module
{
  public:
    Branding();
    ~Branding();

    const char *get_name() override
    {
        return "branding";
    }

  private:
    static void DrawBranding(int localClientNum);
    static void UI_DrawBuildNumber_Hook(int localClientNum);
};
} // namespace mp
} // namespace iw4
