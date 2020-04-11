#ifndef __PHYSICS_H__
#define __PHYSICS_H__

enum Physics_Shape_Type {
    PHYSICS_SHAPE_BOX,
};

constexpr uint phys_group_player = 2;
constexpr uint phys_group_wall = 4;
constexpr uint phys_group_badguy = 8;

struct Physics_Box {
    Vec2 size = Vec2(123, 123);
};

struct Extents {
    float top = 0.0f;
    float left = 0.0f;
    float bottom = 0.0f;
    float right = 0.0f;
};

struct Edge {
    Vec2 a, b;
};

struct Collision_Filter {
    uint groups = (uint)1;
    uint mask = (uint)~0;
};

struct Physics_Object {
    Vec2 position = Vec2(0, 0);
    Vec2 velocity = Vec2(0, 0);
    Vec2 goal_velocity = Vec2(0, 0);
    float velocity_ramp_speed = 0.0f;
    
    Extents extents;
    Vec2 size = Vec2(0, 0);
    float hh = 0.0f;
    float hw = 0.0f;
    Edge edges[4];

    void set_mass(float m) { mass = m; if(m == 0) inv_mass = 0; else inv_mass = 1.0f / m; }
    float mass = 0.0f;
    float inv_mass = 0.0f;
    float restitution = 0.0f;

    bool colliding = false;
    bool deleted = false;

    uint groups = (uint)1;
    uint mask = (uint)~0;
};

void physics_init();
Physics_Object *physics_add_object();
void physics_remove_object(Physics_Object *po);
void physics_step_world(float dt);

void physics_render_debug(Physics_Object *po);

struct Raycast_Hit {
    Vec2 point = Vec2(0, 0);
    float alpha = 0.0f;
};

bool raycast(Vec2 a, Vec2 b, Raycast_Hit *hit, Collision_Filter filter);

#endif