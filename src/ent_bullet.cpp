#include "precompiled.h"

class Bullet : public Entity {
    void spawn() {
        set_texture("data/textures/default.png");
    }
};

declare_entity_type(Bullet, "ent_bullet", ENTITY_BULLET);