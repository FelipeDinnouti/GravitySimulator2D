#include <iostream>
#include <vector>
#include <cmath>

#include "raylib.h"
#include "rlgl.h"


#define RAYGUI_IMPLEMENTATION
#include "../libs/raygui.h"
#include "../libs/rcamera.h"
#include "../libs/raymath.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

// Implementing operator overloading 

// inline Vector2 operator+(const Vector2& a, const Vector2& b) {
//     return Vector2Add(a, b);
// }

// inline Vector2 operator-(const Vector2& a, const Vector2& b) {
//     return Vector2Subtract(a, b);
// }

// inline Vector2 operator*(const Vector2& a, float scalar) {
//     return Vector2Scale(a, scalar);
// }

inline Vector2 operator*(const Vector2& a, int scalar) {
 return Vector2Scale(a, scalar);
}

inline Vector2 operator*(const Vector2& a, double scalar) {
 return Vector2Scale(a, scalar);
}

class Actor {
    private:
        double mass;
        int radius;
    public:
        Vector2 position;
        Vector2 velocity;
        Color color;

        double rotational_velocity;
        double rotation;

        Actor(Vector2 t_position, Vector2 initial_velocity, int t_radius, Color t_color) {
            position = t_position;
            velocity = initial_velocity;
            radius = t_radius;
            color = t_color;

            radius = t_radius;
            mass = radius*radius;
        } 

        void set_radius(int t_radius) {
            radius = t_radius;
            mass = radius*radius;
        }
        int get_radius() {
            return radius;
        }
        double get_mass() {
            return mass;
        }
};

class MyCamera {
    private: 
        Vector2 target_position;
        Vector2 position;           
    public: 
        Vector2 attached_vector;
        bool attached; // If attached, it lerps to target_position
        float attach_factor = 0.1f; // Lerp parameter
        double zoom;

        MyCamera(double t_zoom) {
            zoom = t_zoom;
        }
        void update(float delta=1.0f) {
            position = Vector2Lerp(position, target_position, attach_factor*delta);
        }
        void set_position(float x, float y) {
            position.x = x;
            position.y = y;
            target_position.x = x;
            target_position.y = y;
        }
        Vector2 get_position() {
            return position;
        }
        void set_target_position(float x, float y) {
            target_position.x = x;
            target_position.y = y;
        }
        Vector2* get_target_position() {
            return &target_position;
        }

};

int main() {
    const int DEFAULT_WINDOW_WIDTH = 600;
    const int DEFAULT_WINDOW_HEIGHT = 800;
    int window_width = 600;
    int window_height = 800;
    float gravitational_constant = 0.8f;
    float create_actor_size = 50.0f;
    Vector2 window_center = Vector2{window_width/2, window_height/2};

    // Initilization
    InitWindow(window_width, window_height, "My First Project");
    SetTargetFPS(60);

    DisableCursor(); // Captures the mouse

    // Game initialization
    using namespace std;
    vector<Actor*> actors = {};

    // actors.push_back(new Actor(Vector2{400, 400}, Vector2{0.0f, -1.0f}, 10, BLUE));
    actors.push_back(new Actor(Vector2{200, 400}, Vector2{0.0f, 2.0f}, 10, BLUE));
    actors.push_back(new Actor(Vector2{300, 300}, Vector2{-2.0f, -0.0f}, 10, BLUE));
    actors.push_back(new Actor(Vector2{300, 500}, Vector2{2.0f, 0.0f}, 10, BLUE));

    Actor* central_actor = new Actor(Vector2{300, 400}, Vector2{0, 0}, 20, RED);
    actors.push_back(central_actor);     

    MyCamera* current_camera = new MyCamera(1.0f);
    current_camera->set_position(300, 400);

    // Used as a cursor for throwing objects
    Actor* reference_actor = new Actor(Vector2{300,500}, Vector2{0.0f, 0.0f}, 5, WHITE);
    bool is_throwing = false;// Is d pressed

    // Help menu
    bool help_menu_active;

    // Focus
    bool is_focusing = false;
    int focus_target = 0;

    while (!WindowShouldClose()) {
        // Toggle focus 
        if (IsKeyPressed(KEY_S)) {
            is_focusing = !is_focusing;
        }

        // Change focus
        if (IsKeyPressed(KEY_TAB)) {
            focus_target++;
            if (focus_target>=(actors.size())) {
                focus_target = 0;
            }
        }

        // Toggle help menu
        if (IsKeyPressed(KEY_H)) {
            help_menu_active = !help_menu_active;
        }

        // Toggle mouse capture with F
        if (IsKeyPressed(KEY_F)) {
            if (IsCursorHidden()) EnableCursor();
            else DisableCursor();
        }

        // Toggle fullscren with G
        if (IsKeyPressed(KEY_G)) {
            if (IsWindowFullscreen()) {
                window_width = DEFAULT_WINDOW_WIDTH;
                window_height = DEFAULT_WINDOW_HEIGHT;
                window_center = Vector2{window_width/2, window_height/2};
            } else {
                int current_monitor = GetCurrentMonitor();
                window_width = GetMonitorWidth(current_monitor);
                window_height = GetMonitorHeight(current_monitor);
                window_center = Vector2{(float) (window_width/2),(float) (window_height/2)};
            }

            ToggleFullscreen();

            cout << window_width << ", " << window_height << "\n";
        }

        if (IsKeyPressed(KEY_D)) {
           is_throwing = true;
           reference_actor->position = current_camera->get_position(); // IS IT STORING AS REFERENCE?
           reference_actor->set_radius(create_actor_size); 
        }
        if (IsKeyReleased(KEY_D)) {
            is_throwing = false;
            actors.push_back(new Actor(reference_actor->position, ((reference_actor->position-current_camera->get_position())/reference_actor->get_mass())*80, create_actor_size, PURPLE));
        }

        // Physics Update

        // Calculating gravitational attraction
        for (Actor* actor : actors) {
            for (Actor* other_actor : actors) {
                Vector2 position_difference = other_actor->position-actor->position;
                // Equation is F = (m1*m2/r²)*G
                double distance = Vector2LengthSqr(position_difference); // distance squared - r²

                if (distance == 0) continue;

                double mass_product = actor->get_mass()*other_actor->get_mass(); // m1 * m2 
                double force = gravitational_constant*(mass_product/distance);
                double acceleration = force/actor->get_mass(); // F=m*a  a=F/m

                Vector2 acceleration_vector = Vector2Normalize(position_difference)*acceleration;
                actor->velocity += acceleration_vector;
            }
        }

        // Updating camera position first
        if (IsCursorHidden()) {
            Vector2 new_position = *current_camera->get_target_position() + GetMouseDelta();
            current_camera->set_target_position(new_position.x, new_position.y);
        }

        if (is_focusing) {
            current_camera->set_target_position(actors[focus_target]->position.x, actors[focus_target]->position.y);
        }

        current_camera->update();

        // Calculating collision after all movement processing has been done (forgive the double for for)
        for (Actor* actor: actors) {
            for (Actor* other_actor : actors) {
                Vector2 position_difference = other_actor->position-actor->position;
                double distance = Vector2Length(position_difference); // distance squared - r²
                if (distance>(actor->get_radius()+other_actor->get_radius())) continue;

                // Get contact point of collision
                Vector2 collision_normal = Vector2Normalize(position_difference);
                Vector2 a_contact_point = collision_normal*(-actor->get_radius());
                Vector2 b_contact_point = collision_normal*(other_actor->get_radius());
            
                // Get relative velocities, taking into account angular velocity
                Vector2 vrel_a = Vector2{-a_contact_point.y, a_contact_point.x}*actor->rotational_velocity;
                Vector2 vrel_b = Vector2{-b_contact_point.y, b_contact_point.x}*other_actor->rotational_velocity;

                Vector2 v_contact_a = actor->velocity+vrel_a;
                Vector2 v_contact_b = other_actor->velocity+vrel_b;

                // Normal component of relative velocity
                float vrel_normal = Vector2DotProduct(collision_normal, v_contact_b-v_contact_a);

                if (vrel_normal >= 0) continue;

                double denominator = ((1/actor->get_mass()) + (1/other_actor->get_mass()));
                double impulse = -(1+0.8) * vrel_normal/denominator;

                actor->velocity -= (collision_normal*impulse)/actor->get_mass();
                other_actor->velocity += (collision_normal*impulse)/other_actor->get_mass();
            }   
        }

        // Applying velocity
        for (Actor* actor : actors) {
            actor->position += actor->velocity;         
        }

        BeginDrawing();
        ClearBackground(BLACK);
        
        // Drawing physics actors
        for (Actor* actor: actors) {
            // cout << actor->position.x << ", " << actor->position.y << "\n";
            Vector2 relative_position = actor->position-current_camera->get_position(); // To camera
            Vector2 screen_position = relative_position+window_center;

            DrawCircle(screen_position.x, screen_position.y, actor->get_radius(), actor->color);
            
            // Actual direction (rotation direction)
            Vector2 rotation_direction = {(float) cos(actor->rotation), (float) sin(actor->rotation)};
            rotation_direction *= actor->get_radius();
            rotation_direction += actor->position;
            
            // DrawLine(actor->position.x, actor->position.y, rotation_direction.x, rotation_direction.y, WHITE);

            // Velocity direction

            Vector2 direction_line_start = Vector2Normalize(actor->velocity);
            Vector2 direction_line_end = direction_line_start*60;

            direction_line_start *= 5;

            // Transform to actor position
            direction_line_end += actor->position;
            direction_line_start += actor->position;

            // DrawLine(direction_line_start.x, direction_line_start.y, direction_line_end.x, direction_line_end.y, actor->color);
        }

        // Drawing reference actor
        if (is_throwing) {

            Vector2 relative_position = reference_actor->position-current_camera->get_position(); // To camera
            Vector2 screen_position = relative_position+window_center;
            DrawCircle(screen_position.x, screen_position.y, reference_actor->get_radius(), reference_actor->color);
            DrawLine(screen_position.x, screen_position.y, window_center.x, window_center.y, WHITE);

        }

        // Cursor
        DrawLine(window_center.x-5, window_center.y, window_center.x+5, window_center.y, WHITE);
        DrawLine(window_center.x, window_center.y-5, window_center.x, window_center.y+5, WHITE);

        GuiSliderBar((Rectangle){ window_width-250, 0, 200, 20 }, "Gravitational Constant", TextFormat("%.2f", gravitational_constant), &gravitational_constant, 0, 1);
        GuiSliderBar((Rectangle){ window_width-250, 40, 200, 20 }, "Actor Size", TextFormat("%.2f", create_actor_size), &(create_actor_size), 0, 200);

        DrawText("Press H for help menu", 5, 30, 10, GRAY);

        if (help_menu_active) {
            DrawText("D (hold) - Create and throw an actor", 10, 45, 12, WHITE);
            DrawText("F - Toggle mouse lock", 10, 60, 12, WHITE);
            DrawText("G - Fullscreen", 10, 75, 12, WHITE);
            DrawText("Tab - Change Focus", 10, 90, 12, WHITE);
            DrawText("S - Toggle Focus", 10, 105, 12, WHITE);
            
        }

        if (is_focusing) {
            DrawText(TextFormat("FOCUSING ON ACTOR: %d", focus_target), (window_width/2)-60, 10, 16, WHITE);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}