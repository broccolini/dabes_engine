#include "../core/engine.h"
#include "ortho_chipmunk_scene.h"

int OrthoChipmunkScene_create_space(Scene *scene, Engine *engine);

int OrthoChipmunkScene_init(struct Scene *scene, Engine *engine) {
    scene->music = Music_load("media/music/Climb.aif",
                              "media/music/Climb_Loop.aif");
    Music_play(scene->music);
    scene->camera = Camera_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    Scene_load_tile_map(scene, engine, "media/tilemaps/fat.tmx", 0);
    scene->_(start)(scene, engine);

    return 1;
}

void OrthoChipmunkScene_start(struct Scene *scene, Engine *engine) {
    if (scene->started) return;
    assert(scene->world == NULL);
    assert(scene->entities == NULL);
    scene->entities = List_create();

    Scene_configure(scene, engine);
    Scene_reset_camera(scene);

    OrthoChipmunkScene_create_space(scene, engine);

    GameEntity_assign_controller(scene->entities->first->value,
          engine->input->controllers[0]);

    scene->started = 1;
}

void OrthoChipmunkScene_stop(struct Scene *scene, Engine *engine) {
    if (!scene->started) return;
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        GameEntity_destroy(entity);
    }

    List_destroy(scene->entities);
    scene->entities = NULL;

    if (scene->space) {
      cpSpaceFree(scene->space);
      scene->space = NULL;
    }

    Parallax_destroy(scene->parallax);

    engine->physics->accumulator = 0;
    scene->started = 0;
}

void OrthoChipmunkScene_destroy(struct Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to destroy");

    scene->_(stop)(scene, engine);

    Music_destroy(scene->music);
    Camera_destroy(scene->camera);

    if (scene->tile_map) {
        TileMap_destroy(scene->tile_map);
    }

    free(scene);
error:
    return;
}

void OrthoChipmunkScene_update(struct Scene *scene, Engine *engine) {
    if (engine->frame_ticks > 100) engine->frame_ticks = 100;

    unsigned int accumulation = engine->frame_ticks % engine->physics->max_dt;
    engine->physics->accumulator += accumulation;
    int integration = engine->frame_ticks - accumulation;
    int i = 0;
    for (i = 0; i < integration; i += engine->physics->max_dt) {
      cpSpaceStep(scene->space, engine->physics->max_dt / 1000.0);
    }

    while (engine->physics->accumulator >= engine->physics->max_dt) {
      cpSpaceStep(scene->space, engine->physics->max_dt / 1000.0);
      engine->physics->accumulator -= engine->physics->max_dt;
    }

    Camera_track(scene->camera);
}

void OrthoChipmunkScene_render(struct Scene *scene, Engine *engine) {
    Graphics *graphics = ((Engine *)engine)->graphics;

    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *tshader = Graphics_get_shader(graphics, "tilemap");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene->parallax) Parallax_render(scene->parallax, engine->graphics);

    Graphics_project_camera(graphics, scene->camera);

    Graphics_use_shader(graphics, tshader);
    TileMap_render(scene->tile_map, graphics, DEFAULT_PPM * 2.0);

    Graphics_use_shader(graphics, dshader);
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *thing = current->value;
        if (thing == NULL) break;
        GameEntity_render(thing, engine);
    }

    // Camera debug
    if (scene->debug_camera)
        Camera_debug(scene->camera, engine->graphics);

    // Draw the grid
    if (scene->draw_grid && scene->world)
        Scene_draw_debug_grid(scene, graphics);
}

void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine) {
    Input *input = engine->input;
    if (input->cam_reset) {
        Scene_reset_camera(scene);
    }
    scene->camera->scale += 0.02 * input->cam_zoom;
    if (scene->camera->scale < 0) scene->camera->scale = 0;

    scene->camera->rotation_radians += 2 * input->cam_rotate * M_PI / 180;

    if (input->cam_debug) scene->debug_camera = !(scene->debug_camera);
    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);

    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        GameEntity_control(entity, input);
    }
}

int collision_begin_cb(cpArbiter *arb, cpSpace *UNUSED(space), void *UNUSED(data)) {
    cpBody *eBody, *tBody;
    cpArbiterGetBodies(arb, &eBody, &tBody);
    GameEntityStateData *state_data = cpBodyGetUserData(eBody);
    state_data->on_ground++;
    return 1;
}

void collision_seperate_cb(cpArbiter *arb, cpSpace *UNUSED(space), void *UNUSED(data)) {
    cpBody *eBody, *tBody;
    cpArbiterGetBodies(arb, &eBody, &tBody);
    GameEntityStateData *state_data = cpBodyGetUserData(eBody);
    state_data->on_ground--;
    if (state_data->on_ground < 0) state_data->on_ground = 0;
}

int OrthoChipmunkScene_create_space(Scene *scene, Engine *engine) {
    check_mem(scene);
    check_mem(engine);

    scene->space = cpSpaceNew();
    cpVect gravity = {0, 9.8};
    cpSpaceSetGravity(scene->space, gravity);

    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeEntity,
                               OCSCollisionTypeTile, collision_begin_cb, NULL,
                               NULL, collision_seperate_cb, NULL);

    int i = 0;
    LIST_FOREACH(scene->entities, first, next, current) {
      GameEntity *entity = current->value;
      
      GameEntityStateData *state_data = calloc(1, sizeof(GameEntityStateData));
      entity->state = state_data;
      
      cpVect center = {entity->config.center.x,
                       entity->config.center.y};
      
      float moment = cpMomentForBox(entity->config.mass,
                                    entity->config.size.w,
                                    entity->config.size.h);
      cpBody *fixture = cpBodyNew(entity->config.mass, moment);

      cpShape *entity_shape = cpBoxShapeNew(fixture, entity->config.size.w,
                                            entity->config.size.h);
      entity_shape->collision_type = OCSCollisionTypeEntity;
      cpShapeSetFriction(entity_shape, entity->config.edge_friction);
      cpSpaceAddShape(scene->space, entity_shape);
      cpSpaceAddBody(scene->space, fixture);
      cpBodySetPos(fixture, center);
      cpBodySetAngle(fixture, entity->config.rotation);
      cpBodySetUserData(fixture, state_data);

      entity->physics_shape.shape = entity_shape;
      entity->physics_shape.shape_type = GameEntityPhysicsShapeTypeCPShape;
      i++;
    }

    if (scene->tile_map) {
        TileMapLayer *base_layer = DArray_get(scene->tile_map->layers, 0);
        cpBody *map_body = cpSpaceGetStaticBody(scene->space);
        float grid_size = PHYS_DEFAULT_GRID_SIZE;
        int i = 0;
        for (i = 0; i < base_layer->gid_count; i++) {
            uint32_t gid = base_layer->tile_gids[i];
            if (gid == 0) continue;

            // TilesetTile *tile = TileMap_resolve_tile_gid(scene->tile_map, gid);
            int col = i % scene->tile_map->cols;
            int row = i / scene->tile_map->cols;
            cpVect tile_verts[4] = {
              { grid_size * col, grid_size * row + grid_size },
              { grid_size * col + grid_size, grid_size * row + grid_size },
              { grid_size * col + grid_size, grid_size * row },
              { grid_size * col, grid_size * row },
            };
            cpShape *tile_shape = cpPolyShapeNew(map_body, 4, tile_verts,
                                                cpvzero);
            cpShapeSetFriction(tile_shape, 0.5);
            tile_shape->collision_type = OCSCollisionTypeTile;
            cpSpaceAddStaticShape(scene->space, tile_shape);
        }
    }

    return 1;
error:
    return 0;
}


SceneProto OrthoChipmunkSceneProto = {
    .init = OrthoChipmunkScene_init,
    .start = OrthoChipmunkScene_start,
    .stop = OrthoChipmunkScene_stop,
    .destroy = OrthoChipmunkScene_destroy,
    .update = OrthoChipmunkScene_update,
    .render = OrthoChipmunkScene_render,
    .control = OrthoChipmunkScene_control
};
