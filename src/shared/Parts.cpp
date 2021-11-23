// automatically converted

#include "Parts.hpp"
#include "common/PhysFSExt.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include <physfs.h>

// clang-format off
#include "misc/GlobalVariableStorage.cpp"
// clang-format on

void particlesystem::doverlettimestep()
{
    std::int32_t i;

    for (i = 1; i <= num_particles; i++)
        if (active[i])
            verlet(i);
    satisfyconstraints();
}

void particlesystem::doverlettimestepfor(std::int32_t i, std::int32_t j)
{
    verlet(i);
    satisfyconstraintsfor(j);
}

void particlesystem::doeulertimestepfor(std::int32_t i)
{
    euler(i);
}

void particlesystem::doeulertimestep()
{
    std::int32_t i;

    for (i = 1; i <= num_particles; i++)
        if (active[i])
            euler(i);
}

void particlesystem::euler(std::int32_t i)
{
    tvector2 temppos, s;

    // Accumulate Forces
    forces[i].y = forces[i].y + gravity;
    temppos = pos[i];

    vec2scale(s, forces[i], oneovermass[i]);
    vec2scale(s, s, sqr(timestep));

    velocity[i] = vec2add(velocity[i], s);
    pos[i] = vec2add(pos[i], velocity[i]);
    vec2scale(velocity[i], velocity[i], edamping);
    oldpos[i] = temppos;

    forces[i].x = 0;
    forces[i].y = 0;
}

void particlesystem::verlet(std::int32_t i)
{
    tvector2 temppos, s1, s2, d;

    // Accumulate Forces
    forces[i].y = forces[i].y + gravity;
    temppos = pos[i];

    // Pos[I]:= 2 * Pos[I] - OldPos[I] + Forces[I]{ / Mass} * TimeStep * TimeStep;  {Verlet
    // integration}
    vec2scale(s1, pos[i], 1.0 + vdamping);
    vec2scale(s2, oldpos[i], vdamping);

    d = vec2subtract(s1, s2);
    vec2scale(s1, forces[i], oneovermass[i]);
    vec2scale(s2, s1, sqr(timestep));

    pos[i] = vec2add(d, s2);
    oldpos[i] = temppos;

    forces[i].x = 0;
    forces[i].y = 0;
}

void particlesystem::satisfyconstraints()
{
    std::int32_t i;
    tvector2 delta, d;
    float deltalength, diff;

    if (constraintcount > 0)
        for (i = 1; i <= constraintcount; i++)
        {
            constraint &with = constraints[i];
            if (with.active)
            {
                diff = 0;
                delta = vec2subtract(pos[with.partb], pos[with.parta]);
                deltalength = sqrt(vec2dot(delta, delta));
                if (deltalength != 0)
                    diff = (float)((deltalength - with.restlength)) / deltalength;
                if (oneovermass[with.parta] > 0)
                {
                    vec2scale(d, delta, 0.5 * diff);
                    pos[with.parta] = vec2add(pos[with.parta], d);
                }
                if (oneovermass[with.partb] > 0)
                {
                    vec2scale(d, delta, 0.5 * diff);
                    pos[with.partb] = vec2subtract(pos[with.partb], d);
                }
            }
        }
}

void particlesystem::satisfyconstraintsfor(std::int32_t i)
{
    tvector2 delta, d;
    float deltalength, diff;

    {
        constraint &with = constraints[i];

        diff = 0;
        delta = vec2subtract(pos[with.partb], pos[with.parta]);
        deltalength = sqrt(vec2dot(delta, delta));
        if (deltalength != 0)
            diff = (float)((deltalength - with.restlength)) / deltalength;
        if (oneovermass[with.parta] > 0)
        {
            vec2scale(d, delta, 0.5 * diff);
            pos[with.parta] = vec2add(pos[with.parta], d);
        }
        if (oneovermass[with.partb] > 0)
        {
            vec2scale(d, delta, 0.5 * diff);
            pos[with.partb] = vec2subtract(pos[with.partb], d);
        }
    }
}

void particlesystem::createpart(tvector2 start, tvector2 vel, float mass, std::int32_t num)
{
    // Num is now the active Part
    active[num] = true;
    pos[num] = start;
    velocity[num] = vel;

    oldpos[num] = start;
    oneovermass[num] = (float)(1) / mass;
}

void particlesystem::makeconstraint(std::int32_t pa, std::int32_t pb, float rest)
{
    constraintcount += 1;
    {
        constraint &with = constraints[constraintcount];

        with.active = true;
        with.parta = pa;
        with.partb = pb;
        with.restlength = rest;
    }
}

void particlesystem::clone(particlesystem other)
{
    std::int32_t i;
    constraint *otherconstraint;

    constraintcount = other.constraintcount;
    partcount = other.partcount;

    std::memcpy(&active, &other.active, partcount * sizeof(active[1]));
    std::memcpy(&pos, &other.pos, partcount * sizeof(pos[1]));
    std::memcpy(&velocity, &other.velocity, partcount * sizeof(velocity[1]));
    std::memcpy(&oldpos, &other.oldpos, partcount * sizeof(oldpos[1]));
    std::memcpy(&oneovermass, &other.oneovermass, partcount * sizeof(oneovermass[1]));

    for (i = 1; i <= constraintcount; i++)
    {
        otherconstraint = &other.constraints[i];
        {
            constraint &with = constraints[i];

            with.active = otherconstraint->active;
            with.parta = otherconstraint->parta;
            with.partb = otherconstraint->partb;
            with.restlength = otherconstraint->restlength;
        }
    }
}

void particlesystem::loadpoobject(const std::string &filename, float scale)
{
    PHYSFS_File *f;
    std::string nm = "";
    std::string x = "";
    std::string y = "";
    std::string z = "";
    std::string a = "";
    std::string b = "";
    tvector2 p, delta, v;
    std::int32_t pa, pb;
    std::int32_t i;

    if (!PHYSFS_exists((pchar)(filename)))
        return;
    v.x = 0;
    v.y = 0;
    i = 0;
    constraintcount = 0;

    /*$I-*/
    f = PHYSFS_openRead((pchar)(filename));

    if (f == nullptr)
        return;

    do
    {
        PhysFS_ReadLn(f, nm); // name
        if (nm != "CONSTRAINTS")
        {
            PhysFS_ReadLn(f, x); // X
            PhysFS_ReadLn(f, y); // Y
            PhysFS_ReadLn(f, z); // Z

            // make object
            p.x = -strtofloat(x) * scale / 1.2;
            p.y = -strtofloat(z) * scale;

            i += 1;
            createpart(p, v, 1, i);
        }
    } while (!(nm == "CONSTRAINTS"));

    partcount = i;

    do
    {                        // CONSTRAINTS
        PhysFS_ReadLn(f, a); // Part A
        if (a == "ENDFILE")
            break;

        PhysFS_ReadLn(f, b); // Part B
        a.erase(0, 1);
        b.erase(0, 1);
        pa = strtoint(a);
        pb = strtoint(b);

        delta = vec2subtract(pos[pa], pos[pb]);
        makeconstraint(pa, pb, sqrt(vec2dot(delta, delta)));
    } while (!(a == "ENDFILE"));

    PHYSFS_close(f);
    /*$I+*/
}

void particlesystem::stopallparts()
{
    std::int32_t i;

    for (i = 1; i <= num_particles; i++)
        if (active[i])
        {
            velocity[i].x = 0;
            velocity[i].y = 0;
            oldpos[i] = pos[i];
        }
}

void particlesystem::destroy()
{
    std::int32_t i;

    for (i = 1; i <= num_particles; i++)
    {
        active[i] = false;
        pos[i].x = 0;
        pos[i].y = 0;
        oldpos[i] = pos[i];
        velocity[i].x = 0;
        velocity[i].y = 0;
        forces[i].x = 0;
        forces[i].y = 0;
    }
    constraintcount = 0;
}
