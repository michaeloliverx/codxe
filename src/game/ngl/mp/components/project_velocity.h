#pragma once
#include <cstdint>

namespace ngl {
namespace mp {

    // Move PV_Config here. In older C++, using an anonymous enum is a
    // "clean" way to define global constants without storage class issues.
    namespace PV_Config {
        static const uint32_t PatchAddr      = 0x8246E1E0;  // Location of clip_velocity (or where project_velocity should be called)
        static const uint32_t StubAddr       = 0x8244C1C8;  // Location of unused function in XEX (find one)
        static const uint32_t CallToStubAddr = 0x82466704;  // Location of calls to unused function
    }

    typedef float vec3_t[3];

    class project_velocity : public Module {
    public:
        project_velocity();
        virtual ~project_velocity();

    private:
        void install_patch();
        project_velocity(const project_velocity&);
        project_velocity& operator=(const project_velocity&);
    };

    void PM_ProjectVelocity(vec3_t in, vec3_t normal, vec3_t out);

} // namespace mp
} // namespace ngl