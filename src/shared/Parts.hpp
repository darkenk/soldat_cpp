#pragma once

#include "Vector.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include <array>
#include <cstdint>
#include <string>

const std::int32_t num_particles = 560;
const float rkv = 0.98;

struct constraint
{
    bool active;
    std::int32_t parta, partb;
    float restlength;
};

class particlesystem
{
  public:
    PascalArray<bool, 1, num_particles> active;
    PascalArray<tvector2, 1, num_particles> pos;
    PascalArray<tvector2, 1, num_particles> velocity;
    PascalArray<tvector2, 1, num_particles> oldpos;
    PascalArray<tvector2, 1, num_particles> forces;
    PascalArray<float, 1, num_particles> oneovermass;
    float timestep;
    float gravity, vdamping, edamping;
    std::int32_t constraintcount;
    std::int32_t partcount;
    PascalArray<constraint, 1, num_particles> constraints;

  public:
    void doverlettimestep();
    void doverlettimestepfor(std::int32_t i, std::int32_t j);
    void doeulertimestep();
    void doeulertimestepfor(std::int32_t i);
    void createpart(tvector2 start, tvector2 vel, float mass, std::int32_t num);
    void makeconstraint(std::int32_t pa, std::int32_t pb, float rest);
    void clone(particlesystem other);
    void loadpoobject(const std::string &filename, float scale);
    void stopallparts();
    void destroy();
    void satisfyconstraints();

  private:
    void verlet(std::int32_t i);
    void euler(std::int32_t i);
    void satisfyconstraintsfor(std::int32_t i);
};
