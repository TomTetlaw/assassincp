#include "precompiled.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

internal Array<Entity *> selected;

internal bool drag_select = false;
internal bool dragging_entities = false;
internal Vec2 drag_start_point;
internal Vec2 drag_size;
internal bool middle_down = false;

ImGuiIO *io = nullptr;

internal void clear_selected() {
	selected.num = 0;
}

void editor_on_level_load() {
    clear_selected();
}

void editor_init() {
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(sys.window, sys.context);
    ImGui_ImplOpenGL3_Init();

    io = &ImGui::GetIO();
}

void editor_shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void editor_render() {
    render_setup_for_ui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if(drag_select) {
        render_box(drag_start_point + (sys.window_size * 0.5f), drag_size);
    }

    debug_string("dragging_entities = %d, drag_select = %d", dragging_entities, drag_select);
}

void editor_update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(sys.window);
    ImGui::NewFrame();

    ImGui::BeginTabBar("Entities");
    if(ImGui::BeginTabItem("Create")) {
        Entity *new_entity = nullptr;
        if(ImGui::Selectable("Wall")) {
            new_entity = create_entity(Wall)->inner;
        }
        if(ImGui::Selectable("Player")) {
            new_entity = create_entity(Player)->inner;
        }
        if(ImGui::Selectable("Parallax")) {
            new_entity = create_entity(Parallax)->inner;
        }
        if(ImGui::Selectable("Floor")) {
            new_entity = create_entity(Floor)->inner;
        }
        if(new_entity) {
            clear_selected();
            selected.append(new_entity);
        }
        ImGui::EndTabItem();
    }
    if(ImGui::BeginTabItem("List")) {
        for(int i = 0; i < entity_manager.entities.max_index; i++) {
            if(!entity_manager.entities[i]) continue;
            Entity *entity = entity_manager.entities[i];
            char string[1024] = {0};
            sprintf_s(string, 1024, "(%d) %s", entity->handle.parity, entity->type_name);
            if(ImGui::Selectable(string)) {
                clear_selected();
                selected.append(entity);
            }
        }
        ImGui::EndTabItem();
    }
    if(ImGui::BeginTabItem("Selected")) {
        for(int i = 0; i < selected.num; i++) {
            char string[1024] = {0};
            sprintf_s(string, 1024, "(%d) %s", selected[i]->handle.parity, selected[i]->type_name);
            if(ImGui::Selectable(string)) {
                Entity *entity = selected[i];
                clear_selected();
                selected.append(entity);
            }
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

    ImGui::Begin("Properties");
    if(selected.num == 1) {
        Entity *entity = selected[0];
        #define drag_float2(m) { float data[2] = { m.x, m.y }; ImGui::DragFloat2(#m, data); m.x = data[0]; m.y = data[1]; }

        if(entity->po) {
            if(entity->grid_aligned) {
                int data[2] = {entity->grid_x, entity->grid_y};
                static int step = 1;
                ImGui::InputScalarN("entity->grid_position", ImGuiDataType_S32, data, 2, &step, nullptr, "%d");
                entity->grid_x = data[0];
                entity->grid_y = data[1];

                data[0] = entity->grid_w;
                data[1] = entity->grid_h;
                ImGui::InputScalarN("entity->grid_size", ImGuiDataType_S32, data, 2, &step, nullptr, "%d");
                entity->grid_w = data[0];
                entity->grid_h = data[1];
            } else {
                drag_float2(entity->po->position);
                drag_float2(entity->po->size);
            }

            drag_float2(entity->po->velocity);
            drag_float2(entity->po->goal_velocity);
            ImGui::DragFloat("entity->po->velocity_ramp_speed", &entity->po->velocity_ramp_speed);
            ImGui::DragFloat("entity->po->mass", &entity->po->mass);
            ImGui::DragFloat("entity->po->restitution", &entity->po->restitution);
        } else {
            drag_float2(entity->position);
            drag_float2(entity->size);
        }

        ImGui::Checkbox("entity->texture_repeat", &entity->texture_repeat);
        ImGui::InputInt("entity->z", &entity->z, 1, 2);

        if(entity->po) {
            ImGui::Checkbox("entity->grid_aligned", &entity->grid_aligned);

            if(entity->grid_aligned) {
                ImGui::LabelText("entity->po->position", "(%f, %f)", v2parms(entity->po->position));
                ImGui::LabelText("entity->po->size", "(%f, %f)", v2parms(entity->po->size));
            } else {
                ImGui::LabelText("entity->grid_position", "(%d, %d)", entity->grid_x, entity->grid_y);
                ImGui::LabelText("entity->grid_w/h", "(%d, %d)", entity->grid_w, entity->grid_h);
            }

            ImGui::LabelText("entity->grid_size", "(%d, %d)", entity->grid_size_x, entity->grid_size_y);

            ImGui::LabelText("entity->po->inv_mass", "%f", entity->po->inv_mass);
            ImGui::LabelText("entity->po->colliding", "%s", entity->po->colliding ? "true" : "false");
            ImGui::LabelText("entity->po->extents", "(%f, %f, %f, %f)", entity->po->extents.top, entity->po->extents.left, entity->po->extents.bottom, entity->po->extents.right);
            ImGui::LabelText("entity->po->hw/hh", "(%f, %f)", entity->po->hw, entity->po->hh);
            ImGui::LabelText("entity->po->edges[0]", "(%f, %f) -> (%f, %f)", v2parms(entity->po->edges[0].a), v2parms(entity->po->edges[0].b));
            ImGui::LabelText("entity->po->edges[1]", "(%f, %f) -> (%f, %f)", v2parms(entity->po->edges[1].a), v2parms(entity->po->edges[1].b));
            ImGui::LabelText("entity->po->edges[2]", "(%f, %f) -> (%f, %f)", v2parms(entity->po->edges[2].a), v2parms(entity->po->edges[2].b));
            ImGui::LabelText("entity->po->edges[3]", "(%f, %f) -> (%f, %f)", v2parms(entity->po->edges[3].a), v2parms(entity->po->edges[3].b));
        } else {
            // @todo: resizing of entities that don't have physics.
        }

        ImGui::LabelText("entity->delete_me", "%s", entity->delete_me ? "true" : "false");

        //ImGui::DragInt("entity->po->groups", &entity->po->groups);
        //ImGui::DragInt("entity->po->mask", &entity->po->mask);
        //ImGui::Text(entity->texture->filename);
        //ImGui::DragInt("entity->classify", &entity->classify);
    }
    ImGui::End();

    //static bool show_demo_window = true;
    //if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
}

bool editor_gui_handle_event(SDL_Event *ev) {
    bool result = ImGui_ImplSDL2_ProcessEvent(ev);
    return io->WantCaptureMouse;
}

internal bool box_intersects_box(Extents a, Extents b) {
    return (a.left <= b.right) && (a.top <= b.bottom) && (a.right >= b.left) && (a.bottom >= b.top);
}

internal void check_for_selected() {
    // if clicked and didn't move mouse
    if(drag_size.x == 0) {
        drag_size.x = 1;
    }
    if(drag_size.y == 0) {
        drag_size.y = 1;
    }

    // if dragged upwards instead of downwards
    if(drag_size.x < 0) {
        drag_start_point.x = drag_start_point.x + drag_size.x;
        drag_size.x = drag_size.x * -1;
    }
    if(drag_size.y < 0) {
        drag_start_point.y = drag_start_point.y + drag_size.y;
        drag_size.y = drag_size.y * -1;
    }

    Extents select_extents;
    select_extents.top = drag_start_point.y;
    select_extents.left = drag_start_point.x;
    select_extents.bottom = drag_start_point.y + drag_size.y;
    select_extents.right = drag_start_point.x + drag_size.x;

    for(int i = 0; i < entity_manager.entities.max_index; i++) {
        if(!entity_manager.entities[i]) continue;
        Entity *entity = entity_manager.entities[i];
        if(entity->po && box_intersects_box(select_extents, entity->po->extents)) {
            selected.append(entity_manager.entities[i]);
        } else {
            // @todo: selection for entities that don't have physics.
        }
    }
}

internal bool point_intersects_box(Vec2 point, Extents box) {
	if(point.x < box.left || point.x > box.right) return false;
	if(point.y < box.top || point.y > box.bottom) return false;
	return true;
}

bool editor_handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {
    if(mouse_button == SDL_BUTTON_MIDDLE) middle_down = down;

    if(mouse_button == SDL_BUTTON_LEFT) {
        bool selected_entities = false;
        for(int i = 0; i < selected.num; i++) {
            if(!selected[i]) continue;
            Entity *entity = selected[i];
            if(entity->po) {
                if(point_intersects_box(cursor_position_world, entity->po->extents)) {
                    selected_entities = true;
                }
            } else {
                // @todo: selection for entities without physics.
            }
        }

        if(down) {
            if(selected_entities) {
                dragging_entities = true;
            } else {
                drag_select = true; 
                drag_start_point = cursor_position;
                drag_size = Vec2(0, 0);
            }
        } else {
            if(dragging_entities) {
                dragging_entities = false;
            } else {
                drag_select = false;
                clear_selected();
                check_for_selected();
                drag_start_point = Vec2(0, 0);
                drag_size = Vec2(0, 0);
            }
        }
    }

    return false;
}

void editor_handle_mouse_move(int relx, int rely) {
    if(drag_select) {
        drag_size = cursor_position - drag_start_point;
    }
    if(dragging_entities) {
        for(int i = 0; i < selected.num; i++) {
            Entity *entity = selected[i];
            if(!entity) continue;
            if(entity->po) {
                entity->po->position = entity->po->position + Vec2(relx, rely);
            } else {
                // @todo: dragging for entities without physics.
            }
        }
    }
    if(middle_down) {
        renderer.camera_position = renderer.camera_position - Vec2(relx, rely);
    }
}

bool editor_handle_key_press(SDL_Scancode scancode, bool down, int mods) {
    if(selected.num == 1 && selected[0]->grid_aligned == true) {
        if(down) {
            if(mods & KEY_MOD_SHIFT) {
                if(scancode == SDL_SCANCODE_LEFT) {
                    selected[0]->grid_w -= 1;
                } else if (scancode == SDL_SCANCODE_RIGHT) {
                    selected[0]->grid_w += 1;
                } else if (scancode == SDL_SCANCODE_UP) {
                    selected[0]->grid_h += 1;
                } else if (scancode == SDL_SCANCODE_DOWN) {
                    selected[0]->grid_h -= 1;
                }  
            } else {              
                if(scancode == SDL_SCANCODE_LEFT) {
                    selected[0]->grid_x -= 1;
                } else if (scancode == SDL_SCANCODE_RIGHT) {
                    selected[0]->grid_x += 1;
                } else if (scancode == SDL_SCANCODE_UP) {
                    selected[0]->grid_y -= 1;
                } else if (scancode == SDL_SCANCODE_DOWN) {
                    selected[0]->grid_y += 1;
                }
            }
        }
    }

    if(scancode == SDL_SCANCODE_DELETE && down) {
        for(int i = 0; i < selected.num; i++) {
            remove_entity(selected[i]);
        }
        clear_selected();
        return true;
    }

    return false;
}

void editor_handle_mouse_wheel(int amount) {
}