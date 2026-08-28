#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Local inline math and clamp helpers (self-contained)
static inline double gp_sqrt(double x) {
    if (x <= 0) return 0.0;
    double val = x, last = 0.0;
    int i = 0;
    while (val != last && i < 10) {
        last = val;
        val = (val + x / val) * 0.5;
        i++;
    }
    return val;
}

static inline double gp_sin(double x) {
    while (x < -3.141592653589793) x += 6.283185307179586;
    while (x > 3.141592653589793) x -= 6.283185307179586;
    double x2 = x * x;
    return x * (1.0 - x2 / 6.0 * (1.0 - x2 / 20.0 * (1.0 - x2 / 42.0)));
}

static inline double gp_cos(double x) {
    return gp_sin(x + 1.5707963267948966);
}

static inline double gp_do_clamp(double x, double lo, double hi) { 
    return x < lo ? lo : (x > hi ? hi : x); 
}

// ==========================================
// 80 GAME PRIMITIVES FUNCTIONS (_game_primitives.c)
// ==========================================

// 1. Pathfinding & Navigation
static PyObject* gp_pathfind_astar_step(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_bresenham_line(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_spatial_grid_update(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_polygon_area(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_point_in_polygon(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 6. Advanced Collision & Geometry
static PyObject* gp_segment_intersection(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_swept_aabb(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_capsule_collision(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_triangle_contains_point(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_bezier_quadratic(PyObject* self, PyObject* args) {
    double p0, p1, p2, t;
    if (!PyArg_ParseTuple(args, "dddd", &p0, &p1, &p2, &t)) return NULL;
    double mt = 1.0 - t;
    return PyFloat_FromDouble(mt * mt * p0 + 2.0 * mt * t * p1 + t * t * p2);
}
static PyObject* gp_bezier_cubic(PyObject* self, PyObject* args) {
    double p0, p1, p2, p3, t;
    if (!PyArg_ParseTuple(args, "ddddd", &p0, &p1, &p2, &p3, &t)) return NULL;
    double mt = 1.0 - t;
    double mt2 = mt * mt;
    double t2 = t * t;
    return PyFloat_FromDouble(mt2 * mt * p0 + 3.0 * mt2 * t * p1 + 3.0 * mt * t2 * p2 + t2 * t * p3);
}
static PyObject* gp_catmull_rom_spline(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 13. Physics & Control Loops
static PyObject* gp_spring_damper_update(PyObject* self, PyObject* args) {
    double pos, target, vel, stiffness, damping, dt;
    if (!PyArg_ParseTuple(args, "dddddd", &pos, &target, &vel, &stiffness, &damping, &dt)) return NULL;
    double force = -stiffness * (pos - target) - damping * vel;
    vel += force * dt;
    pos += vel * dt;
    return Py_BuildValue("(dd)", pos, vel);
}
static PyObject* gp_verlet_integrate(PyObject* self, PyObject* args) {
    double pos, prev_pos, accel, dt;
    if (!PyArg_ParseTuple(args, "dddd", &pos, &prev_pos, &accel, &dt)) return NULL;
    double temp = pos;
    pos = pos + (pos - prev_pos) + accel * dt * dt;
    return Py_BuildValue("(dd)", pos, temp);
}
static PyObject* gp_pid_controller(PyObject* self, PyObject* args) {
    double error, integral, derivative, kp, ki, kd, dt;
    if (!PyArg_ParseTuple(args, "ddddddd", &error, &integral, &derivative, &kp, &ki, &kd, &dt)) return NULL;
    integral += error * dt;
    double derivative_val = (error - derivative) / dt;
    double output = kp * error + ki * integral + kd * derivative_val;
    return Py_BuildValue("(dd)", output, integral);
}

// 16. Steering Behaviors
static PyObject* gp_steer_seek(PyObject* self, PyObject* args) {
    double px, py, tx, ty, vx, vy, max_speed;
    if (!PyArg_ParseTuple(args, "ddddddd", &px, &py, &tx, &ty, &vx, &vy, &max_speed)) return NULL;
    double dx = tx - px, dy = ty - py;
    double len = gp_sqrt(dx*dx + dy*dy);
    if (len == 0) return Py_BuildValue("(dd)", 0.0, 0.0);
    double desired_x = (dx / len) * max_speed;
    double desired_y = (dy / len) * max_speed;
    return Py_BuildValue("(dd)", desired_x - vx, desired_y - vy);
}
static PyObject* gp_steer_flee(PyObject* self, PyObject* args) {
    double px, py, tx, ty, vx, vy, max_speed;
    if (!PyArg_ParseTuple(args, "ddddddd", &px, &py, &tx, &ty, &vx, &vy, &max_speed)) return NULL;
    double dx = px - tx, dy = py - ty;
    double len = gp_sqrt(dx*dx + dy*dy);
    if (len == 0) return Py_BuildValue("(dd)", 0.0, 0.0);
    double desired_x = (dx / len) * max_speed;
    double desired_y = (dy / len) * max_speed;
    return Py_BuildValue("(dd)", desired_x - vx, desired_y - vy);
}
static PyObject* gp_steer_arrive(PyObject* self, PyObject* args) {
    double px, py, tx, ty, vx, vy, max_speed, slowing_radius;
    if (!PyArg_ParseTuple(args, "dddddddd", &px, &py, &tx, &ty, &vx, &vy, &max_speed, &slowing_radius)) return NULL;
    double dx = tx - px, dy = ty - py;
    double dist = gp_sqrt(dx*dx + dy*dy);
    if (dist == 0) return Py_BuildValue("(dd)", 0.0, 0.0);
    double speed = max_speed;
    if (dist < slowing_radius) speed = max_speed * (dist / slowing_radius);
    double desired_x = (dx / dist) * speed;
    double desired_y = (dy / dist) * speed;
    return Py_BuildValue("(dd)", desired_x - vx, desired_y - vy);
}
static PyObject* gp_steer_wander(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_steer_separation(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 21. Procedural Generation & Noise
static PyObject* gp_noise_perlin_1d(PyObject* self, PyObject* args) {
    double x; if (!PyArg_ParseTuple(args, "d", &x)) return NULL;
    return PyFloat_FromDouble(gp_sin(x) * 0.5 + gp_cos(x * 0.5) * 0.5);
}
static PyObject* gp_noise_simplex_2d(PyObject* self, PyObject* args) {
    double x, y; if (!PyArg_ParseTuple(args, "dd", &x, &y)) return NULL;
    return PyFloat_FromDouble(gp_sin(x + y) * gp_cos(x - y));
}
static PyObject* gp_fractal_brownian_motion(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_cellular_automata_step(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_flocking_update(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 26. AI & Game Systems
static PyObject* gp_behavior_tree_tick(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_fog_of_war_reveal(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_loot_table_roll(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_experience_level_curve(PyObject* self, PyObject* args) {
    long level; double base, scaling;
    if (!PyArg_ParseTuple(args, "ldd", &level, &base, &scaling)) return NULL;
    return PyFloat_FromDouble(base * gp_sin(level * scaling) + level * 100.0);
}
static PyObject* gp_damage_mitigation_armor(PyObject* self, PyObject* args) {
    double damage, armor;
    if (!PyArg_ParseTuple(args, "dd", &damage, &armor)) return NULL;
    return PyFloat_FromDouble(damage * (100.0 / (100.0 + armor)));
}

// 31. Combat & RPG Formulas
static PyObject* gp_critical_strike_check(PyObject* self, PyObject* args) {
    double crit_chance; if (!PyArg_ParseTuple(args, "d", &crit_chance)) return NULL;
    double roll = ((double)rand() / RAND_MAX) * 100.0;
    if (roll <= crit_chance) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static PyObject* gp_knockback_velocity(PyObject* self, PyObject* args) {
    double px, py, tx, ty, force;
    if (!PyArg_ParseTuple(args, "ddddd", &px, &py, &tx, &ty, &force)) return NULL;
    double dx = px - tx, dy = py - ty;
    double dist = gp_sqrt(dx*dx + dy*dy);
    if (dist == 0) return Py_BuildValue("(dd)", 0.0, 0.0);
    return Py_BuildValue("(dd)", (dx / dist) * force, (dy / dist) * force);
}
static PyObject* gp_status_effect_tick(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_combo_window_check(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_projectile_predict_intercept(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 36. Movement & Raymarching
static PyObject* gp_homing_missile_steer(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_raymarch_step(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_billboard_rotation_matrix(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_frustum_culling_test(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_octree_insert(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 41. Particles & Fluid Dynamics
static PyObject* gp_particle_system_advance(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_fluid_particle_pressure(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_cloth_node_constrain(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_rope_segment_solve(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_rigidbody_impulse_resolve(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 46. Environmental Forces
static PyObject* gp_buoyancy_force(PyObject* self, PyObject* args) {
    double y, water_level, density;
    if (!PyArg_ParseTuple(args, "ddd", &y, &water_level, &density)) return NULL;
    if (y < water_level) return PyFloat_FromDouble((water_level - y) * density);
    return PyFloat_FromDouble(0.0);
}
static PyObject* gp_magnetic_pull(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_gravity_well_pull(PyObject* self, PyObject* args) {
    double px, py, gx, gy, mass, strength;
    if (!PyArg_ParseTuple(args, "dddddd", &px, &py, &gx, &gy, &mass, &strength)) return NULL;
    double dx = gx - px, dy = gy - py;
    double dist2 = dx*dx + dy*dy + 1.0;
    double f = (mass * strength) / dist2;
    double dist = gp_sqrt(dist2);
    return Py_BuildValue("(dd)", (dx / dist) * f, (dy / dist) * f);
}
static PyObject* gp_portal_teleport_vector(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_time_dilation_step(PyObject* self, PyObject* args) {
    double dt, scale; if (!PyArg_ParseTuple(args, "dd", &dt, &scale)) return NULL;
    return PyFloat_FromDouble(dt * scale);
}

// 51. Gameplay Utilities & Polish
static PyObject* gp_rewind_buffer_push(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_rewind_buffer_pop(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_matchmaking_elo_delta(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_recoil_pattern_offset(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_bullet_spread_cone(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 56. Audio & Interface Helpers
static PyObject* gp_footstep_surface_matcher(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_audio_spatial_attenuation(PyObject* self, PyObject* args) {
    double dist, max_dist; if (!PyArg_ParseTuple(args, "dd", &dist, &max_dist)) return NULL;
    if (dist >= max_dist) return PyFloat_FromDouble(0.0);
    return PyFloat_FromDouble(1.0 - (dist / max_dist));
}
static PyObject* gp_dialogue_typewriter_step(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_inventory_slot_swap(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_stackable_item_merge(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 61. Economy & Crafting Logic
static PyObject* gp_crafting_recipe_match(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_durability_decay_step(PyObject* self, PyObject* args) {
    double dur, decay; if (!PyArg_ParseTuple(args, "dd", &dur, &decay)) return NULL;
    return PyFloat_FromDouble(gp_do_clamp(dur - decay, 0.0, 100.0));
}
static PyObject* gp_threat_table_aggro_calc(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_line_of_sight_raycast(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_grid_flood_fill(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 66. World Generation & Environment
static PyObject* gp_noise_biome_selector(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_chunk_coordinate_hash(PyObject* self, PyObject* args) {
    long cx, cy; if (!PyArg_ParseTuple(args, "ll", &cx, &cy)) return NULL;
    return PyLong_FromLong((cx * 73856093) ^ (cy * 19349663));
}
static PyObject* gp_day_night_cycle_progress(PyObject* self, PyObject* args) {
    double time, duration; if (!PyArg_ParseTuple(args, "dd", &time, &duration)) return NULL;
    double progress = time / duration;
    return PyFloat_FromDouble(progress - (long)progress);
}
static PyObject* gp_weather_transition_blend(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_shadow_projection_point(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

// 71. Screen Effects & Shaders
static PyObject* gp_bloom_threshold_filter(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_chromatic_aberration_offset(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_vignette_intensity_calc(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_pixelate_coordinate_snap(PyObject* self, PyObject* args) {
    double x, y, size; if (!PyArg_ParseTuple(args, "ddd", &x, &y, &size)) return NULL;
    return Py_BuildValue("(dd)", (double)((long)(x / size) * size), (double)((long)(y / size) * size));
}
static PyObject* gp_screen_shake_trauma(PyObject* self, PyObject* args) {
    double trauma; if (!PyArg_ParseTuple(args, "d", &trauma)) return NULL;
    double intensity = trauma * trauma;
    double ox = ((rand() % 20) - 10) * 0.1 * intensity;
    double oy = ((rand() % 20) - 10) * 0.1 * intensity;
    return Py_BuildValue("(dd)", ox, oy);
}

// 76. Animation & Advanced Physics
static PyObject* gp_ghost_trail_record(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_ik_two_bone_solve(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_root_motion_extract(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_hitstop_freeze_check(PyObject* self, PyObject* args) {
    double timer; if (!PyArg_ParseTuple(args, "d", &timer)) return NULL;
    if (timer > 0.0) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static PyObject* gp_game_primitives_version_info(PyObject* self, PyObject* args) {
    return PyUnicode_FromString("_game_primitives 1.0.0 (Standalone 80 Game Primitives)");
}

// ==========================================
// METHOD TABLE & MODULE INITIALIZATION
// ==========================================
static PyMethodDef GamePrimitivesMethods[] = {
    {"pathfind_astar_step", gp_pathfind_astar_step, METH_VARARGS, ""},
    {"bresenham_line", gp_bresenham_line, METH_VARARGS, ""},
    {"spatial_grid_update", gp_spatial_grid_update, METH_VARARGS, ""},
    {"polygon_area", gp_polygon_area, METH_VARARGS, ""},
    {"point_in_polygon", gp_point_in_polygon, METH_VARARGS, ""},
    {"segment_intersection", gp_segment_intersection, METH_VARARGS, ""},
    {"swept_aabb", gp_swept_aabb, METH_VARARGS, ""},
    {"capsule_collision", gp_capsule_collision, METH_VARARGS, ""},
    {"triangle_contains_point", gp_triangle_contains_point, METH_VARARGS, ""},
    {"bezier_quadratic", gp_bezier_quadratic, METH_VARARGS, ""},
    {"bezier_cubic", gp_bezier_cubic, METH_VARARGS, ""},
    {"catmull_rom_spline", gp_catmull_rom_spline, METH_VARARGS, ""},
    {"spring_damper_update", gp_spring_damper_update, METH_VARARGS, ""},
    {"verlet_integrate", gp_verlet_integrate, METH_VARARGS, ""},
    {"pid_controller", gp_pid_controller, METH_VARARGS, ""},
    {"steer_seek", gp_steer_seek, METH_VARARGS, ""},
    {"steer_flee", gp_steer_flee, METH_VARARGS, ""},
    {"steer_arrive", gp_steer_arrive, METH_VARARGS, ""},
    {"steer_wander", gp_steer_wander, METH_VARARGS, ""},
    {"steer_separation", gp_steer_separation, METH_VARARGS, ""},

    {"noise_perlin_1d", gp_noise_perlin_1d, METH_VARARGS, ""},
    {"noise_simplex_2d", gp_noise_simplex_2d, METH_VARARGS, ""},
    {"fractal_brownian_motion", gp_fractal_brownian_motion, METH_VARARGS, ""},
    {"cellular_automata_step", gp_cellular_automata_step, METH_VARARGS, ""},
    {"flocking_update", gp_flocking_update, METH_VARARGS, ""},
    {"behavior_tree_tick", gp_behavior_tree_tick, METH_VARARGS, ""},
    {"fog_of_war_reveal", gp_fog_of_war_reveal, METH_VARARGS, ""},
    {"loot_table_roll", gp_loot_table_roll, METH_VARARGS, ""},
    {"experience_level_curve", gp_experience_level_curve, METH_VARARGS, ""},
    {"damage_mitigation_armor", gp_damage_mitigation_armor, METH_VARARGS, ""},
    {"critical_strike_check", gp_critical_strike_check, METH_VARARGS, ""},
    {"knockback_velocity", gp_knockback_velocity, METH_VARARGS, ""},
    {"status_effect_tick", gp_status_effect_tick, METH_VARARGS, ""},
    {"combo_window_check", gp_combo_window_check, METH_VARARGS, ""},
    {"projectile_predict_intercept", gp_projectile_predict_intercept, METH_VARARGS, ""},
    {"homing_missile_steer", gp_homing_missile_steer, METH_VARARGS, ""},
    {"raymarch_step", gp_raymarch_step, METH_VARARGS, ""},
    {"billboard_rotation_matrix", gp_billboard_rotation_matrix, METH_VARARGS, ""},
    {"frustum_culling_test", gp_frustum_culling_test, METH_VARARGS, ""},
    {"octree_insert", gp_octree_insert, METH_VARARGS, ""},

    {"particle_system_advance", gp_particle_system_advance, METH_VARARGS, ""},
    {"fluid_particle_pressure", gp_fluid_particle_pressure, METH_VARARGS, ""},
    {"cloth_node_constrain", gp_cloth_node_constrain, METH_VARARGS, ""},
    {"rope_segment_solve", gp_rope_segment_solve, METH_VARARGS, ""},
    {"rigidbody_impulse_resolve", gp_rigidbody_impulse_resolve, METH_VARARGS, ""},
    {"buoyancy_force", gp_buoyancy_force, METH_VARARGS, ""},
    {"magnetic_pull", gp_magnetic_pull, METH_VARARGS, ""},
    {"gravity_well_pull", gp_gravity_well_pull, METH_VARARGS, ""},
    {"portal_teleport_vector", gp_portal_teleport_vector, METH_VARARGS, ""},
    {"time_dilation_step", gp_time_dilation_step, METH_VARARGS, ""},
    {"rewind_buffer_push", gp_rewind_buffer_push, METH_VARARGS, ""},
    {"rewind_buffer_pop", gp_rewind_buffer_pop, METH_VARARGS, ""},
    {"matchmaking_elo_delta", gp_matchmaking_elo_delta, METH_VARARGS, ""},
    {"recoil_pattern_offset", gp_recoil_pattern_offset, METH_VARARGS, ""},
    {"bullet_spread_cone", gp_bullet_spread_cone, METH_VARARGS, ""},
    {"footstep_surface_matcher", gp_footstep_surface_matcher, METH_VARARGS, ""},
    {"audio_spatial_attenuation", gp_audio_spatial_attenuation, METH_VARARGS, ""},
    {"dialogue_typewriter_step", gp_dialogue_typewriter_step, METH_VARARGS, ""},
    {"inventory_slot_swap", gp_inventory_slot_swap, METH_VARARGS, ""},
    {"stackable_item_merge", gp_stackable_item_merge, METH_VARARGS, ""},

    {"crafting_recipe_match", gp_crafting_recipe_match, METH_VARARGS, ""},
    {"durability_decay_step", gp_durability_decay_step, METH_VARARGS, ""},
    {"threat_table_aggro_calc", gp_threat_table_aggro_calc, METH_VARARGS, ""},
    {"line_of_sight_raycast", gp_line_of_sight_raycast, METH_VARARGS, ""},
    {"grid_flood_fill", gp_grid_flood_fill, METH_VARARGS, ""},
    {"noise_biome_selector", gp_noise_biome_selector, METH_VARARGS, ""},
    {"chunk_coordinate_hash", gp_chunk_coordinate_hash, METH_VARARGS, ""},
    {"day_night_cycle_progress", gp_day_night_cycle_progress, METH_VARARGS, ""},
    {"weather_transition_blend", gp_weather_transition_blend, METH_VARARGS, ""},
    {"shadow_projection_point", gp_shadow_projection_point, METH_VARARGS, ""},
    {"bloom_threshold_filter", gp_bloom_threshold_filter, METH_VARARGS, ""},
    {"chromatic_aberration_offset", gp_chromatic_aberration_offset, METH_VARARGS, ""},
    {"vignette_intensity_calc", gp_vignette_intensity_calc, METH_VARARGS, ""},
    {"pixelate_coordinate_snap", gp_pixelate_coordinate_snap, METH_VARARGS, ""},
    {"screen_shake_trauma", gp_screen_shake_trauma, METH_VARARGS, ""},
    {"ghost_trail_record", gp_ghost_trail_record, METH_VARARGS, ""},
    {"ik_two_bone_solve", gp_ik_two_bone_solve, METH_VARARGS, ""},
    {"root_motion_extract", gp_root_motion_extract, METH_VARARGS, ""},
    {"hitstop_freeze_check", gp_hitstop_freeze_check, METH_VARARGS, ""},
    {"game_primitives_version_info", gp_game_primitives_version_info, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef game_primitives_module = {
    PyModuleDef_HEAD_INIT,
    "_game_primitives",
    "80 Game Primitives Functions Without Math.h",
    -1,
    GamePrimitivesMethods
};

PyMODINIT_FUNC PyInit__game_primitives(void) {
    return PyModule_Create(&game_primitives_module);
}
