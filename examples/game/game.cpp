// basic_game_setup.cpp
#include <iostream>
#include <utility>
#include <format>
#include "VHInclude.h"
#include "VEInclude.h"
#include <windows.h>
#undef SendMessage


class MyGame : public vve::System {
public:
    MyGame(vve::Engine& engine) : vve::System("MyGame", engine) {
        m_engine.RegisterCallback({
            {this, -10, "LOAD_LEVEL", [this](Message& msg) { return OnLoadLevel(msg); }},
            {this, 10000, "UPDATE", [this](Message& msg) { return OnUpdate(msg); }},
            {this, -10000, "RECORD_NEXT_FRAME", [this](Message& msg) { return OnRecordNextFrame(msg); }},
            {this, -10, "SDL_KEY_DOWN", [this](Message& message){ return OnKeyDown(message);} },
			{this, -10, "SDL_KEY_REPEAT", [this](Message& message){ return OnKeyDown(message);} },
			// {this, -10, "SDL_KEY_UP", [this](Message& message){ return OnKeyUp(message);} },
			// {this, 0, "SDL_MOUSE_BUTTON_DOWN", [this](Message& message){ return OnMouseButtonDown(message);} },
			// {this, 0, "SDL_MOUSE_BUTTON_UP", [this](Message& message){return OnMouseButtonUp(message);} },
			// {this, 0, "SDL_MOUSE_MOVE", [this](Message& message){ return OnMouseMove(message); } },
			// {this, 0, "SDL_MOUSE_WHEEL", [this](Message& message){ return OnMouseWheel(message); } },
			// {this, 0, "FRAME_END", [this](Message& message){ return OnFrameEnd(message); } }
            });    
    }

    ~MyGame() {};

private:
    enum class PlayerState {
        STATIONARY,
        MOVING
    };

    // enum class DecodeState { NONE, DECODING, COMPLETE };

    vecs::Handle m_cameraHandle{};
    vecs::Handle m_cameraNodeHandle{};
    vecs::Handle m_playerHandle{};
    vecs::Handle m_cypherHandle{};
    vecs::Handle m_buffHandle{};
    float m_volume{100.0f};

    PlayerState m_playerState = PlayerState::STATIONARY;

    std::vector<vecs::Handle> m_cubeHandles;
    int m_cubeCollected = 0;
    glm::vec3 m_cameraOffsetLocal = glm::vec3(0.0f, -5.0f, 5.0f);  

    inline static std::string plane_obj{ "assets/test/plane/plane_t_n_s.obj" };
    inline static std::string plane_mesh{ "assets/test/plane/plane_t_n_s.obj/plane" };
    inline static std::string plane_txt{ "assets/test/plane/grass.jpg" };
    inline static std::string player_obj{ "assets/mini_characters/Models/OBJ format/character-female-a.obj" };
    inline static std::string cube_obj{ "assets/test/crate0/cube.obj" };
    inline static std::string sphere_obj{ "assets/test/sphere/sphere.obj" };

    vec3_t RandomPosition() {
        return vec3_t{ float(rand() % 40 - 20), float(rand() % 40 - 20), 0.5f };
    }

    void GetCamera() {
        if (!m_cameraHandle.IsValid()) {
            auto [handle, camera, parent] = *m_registry.GetView<vecs::Handle, vve::Camera&, vve::ParentHandle>().begin();
            m_cameraHandle = handle; // (orientation)
            m_cameraNodeHandle = parent; // (position)
        }
    }

    void SpawnCubes(int count) {
        for (int i = 0; i < count; ++i) {
            vecs::Handle handle = m_registry.Insert(
                vve::Position{ RandomPosition() },
                vve::Rotation{ mat3_t{1.0f} },
                vve::Scale{ vec3_t{1.0f} }
            );
            m_cubeHandles.push_back(handle);
    
            m_engine.SendMessage(MsgSceneCreate{
                vve::ObjectHandle(handle), vve::ParentHandle{}, vve::Filename{cube_obj}, aiProcess_FlipWindingOrder
            });
        }
    }

    bool OnLoadLevel(Message& message) {
        auto msg = message.template GetData<vve::System::MsgLoadLevel>();
        std::cout << "Loading level: " << msg.m_level << std::endl;
        std::string level = std::string("Level: ") + msg.m_level;

        m_engine.SendMessage(MsgSceneLoad{ vve::Filename{plane_obj}, aiProcess_FlipWindingOrder });

        auto m_handlePlane = m_registry.Insert(
            vve::Position{ {0.0f,0.0f,0.0f } },
            vve::Rotation{ mat3_t { glm::rotate(glm::mat4(1.0f), 3.14152f / 2.0f, glm::vec3(1.0f,0.0f,0.0f)) } },
            vve::Scale{ vec3_t{1000.0f,1000.0f,1000.0f} },
            vve::MeshName{ plane_mesh },
            vve::TextureName{ plane_txt },
            vve::UVScale{ { 1000.0f, 1000.0f } }
        );

        m_engine.SendMessage(MsgObjectCreate{ 
            vve::ObjectHandle(m_handlePlane), vve::ParentHandle{} 
        });

        
        m_playerHandle = m_registry.Insert(
            vve::Position{ {0, 0, 0} },
            vve::Rotation{
                glm::mat3(
                    glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0,0,1)) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1,0,0))
                )
            },
            vve::Scale{ vec3_t{1.0f} }
        );
        
        m_engine.SendMessage(MsgSceneCreate{
            vve::ObjectHandle(m_playerHandle), vve::ParentHandle{}, vve::Filename{player_obj}
        });

        GetCamera();

        m_engine.SendMessage(MsgObjectSetParent{
            vve::ObjectHandle{m_cameraNodeHandle},
            vve::ParentHandle{m_playerHandle}
        });

        
        m_registry.Get<vve::Position&>(m_cameraNodeHandle)() = glm::vec3(0.0f, 1.0f, -4.0f);
        auto& camPos = m_registry.Get<vve::Position&>(m_cameraNodeHandle)();
        glm::vec3 target = m_registry.Get<vve::Position&>(m_playerHandle)();

        glm::mat4 viewMatrix = glm::lookAt(camPos, target, glm::vec3(0.0f, 0.0f, 1.0f));

        // Update camera rotation
        m_registry.Get<vve::Rotation&>(m_cameraHandle)() = glm::mat3(glm::inverse(viewMatrix));

        SpawnCubes(10);
        m_engine.SendMessage(MsgPlaySound{ vve::Filename{"assets/sounds/stardew.wav"}, 2, 100 });
        // SpawnBuff();
        return false;
    }

    bool OnKeyDown(Message message) {
        GetCamera();
    
        // Get component references
        auto [pn, rn, sn, LtoPn] = m_registry.template Get<vve::Position&, vve::Rotation&, vve::Scale&, vve::LocalToParentMatrix>(m_cameraNodeHandle);
        auto [pc, rc, sc, LtoPc] = m_registry.template Get<vve::Position&, vve::Rotation&, vve::Scale&, vve::LocalToParentMatrix>(m_cameraHandle);

        auto& playerPos = m_registry.Get<vve::Position&>(m_playerHandle)();
        auto& playerRot = m_registry.Get<vve::Rotation&>(m_playerHandle)();
    
        int key; 
        float dt;
        if (message.HasType<MsgKeyDown>()) {
            auto msg = message.template GetData<MsgKeyDown>();
            key = msg.m_key;
            dt = msg.m_dt;
            
        } else {
            auto msg = message.template GetData<MsgKeyRepeat>();
            key = msg.m_key;
            dt = msg.m_dt;
        }
    
        float moveSpeed = dt * 5.0f;
        float rotSpeed = glm::radians(90.0f) * dt; // 90 degrees/sec
    
        glm::vec3 moveDir{0.0f};
    
        // Define local movement directions
        glm::vec3 localForward = glm::vec3(0.0f, 0.0f, 1.0f); // +Z
        glm::vec3 localRight   = glm::vec3(1.0f, 0.0f, 0.0f); // +X
    
        // Convert local directions into world directions
        glm::vec3 forward = playerRot * localForward;
        glm::vec3 right   = playerRot * localRight;
        // glm::vec3 c_forward = camNodeRot * localForward;
        // glm::vec3 c_right   = camNodeRot * localRight;

        m_playerState = PlayerState::STATIONARY;

        // for Camera
        glm::vec3 translate(0.0f);
        glm::vec3 axis1(1.0f), axis2(1.0f);
        float angle1 = 0.0f, angle2 = 0.0f;
        int dx = 0, dy = 0;

        switch (key) {
            case SDL_SCANCODE_W: {
                moveDir += forward * moveSpeed;
                m_playerState = PlayerState::MOVING;
                playerPos += moveDir;

                translate = glm::vec3(LtoPn() * LtoPc() * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
                break;
            }
            case SDL_SCANCODE_S: {
                moveDir -= forward * moveSpeed;
                m_playerState = PlayerState::MOVING;
                playerPos += moveDir;

                translate = glm::vec3(LtoPn() * LtoPc() * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
                break;
            }
            case SDL_SCANCODE_A: {
                moveDir += right * moveSpeed;
                m_playerState = PlayerState::MOVING;
                playerPos += moveDir;

                translate = glm::vec3(LtoPn() * LtoPc() * glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f));
                break;
            }
            case SDL_SCANCODE_D: {
                moveDir -= right * moveSpeed;
                m_playerState = PlayerState::MOVING;
                playerPos += moveDir;

                translate = glm::vec3(LtoPn() * LtoPc() * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
                break;
            }
            case SDL_SCANCODE_LEFT: {
                glm::vec3 rotAxis = glm::vec3(0.0f, 1.0f, 0.0f); // rotate around Z
                playerRot = glm::mat3(glm::rotate(glm::mat4(playerRot), rotSpeed, rotAxis));
                dx = -1;
                break;
            }
            case SDL_SCANCODE_RIGHT: {
                glm::vec3 rotAxis = glm::vec3(0.0f, 1.0f, 0.0f); // rotate around Z
                playerRot = glm::mat3(glm::rotate(glm::mat4(playerRot), -rotSpeed, rotAxis));
                dx = 1;
                break;
            }
            case SDL_SCANCODE_SPACE: {
                if (message.HasType<MsgKeyRepeat>()) break;

                if (m_cubeCollected > 0) {
                    glm::vec3 forward = playerRot * glm::vec3(0.0f, 0.0f, 1.0f);
                    glm::vec3 placePos = playerPos + forward * 1.5f + glm::vec3(0.0f, 0.0f, 0.5f); // 1.5 units ahead of player

                    vecs::Handle placedCube = m_registry.Insert(
                        vve::Position{ placePos },
                        vve::Rotation{ mat3_t{1.0f} },
                        vve::Scale{ vec3_t{1.0f} }
                    );

                    m_cubeHandles.push_back(placedCube);

                    m_engine.SendMessage(MsgSceneCreate{
                        vve::ObjectHandle(placedCube), vve::ParentHandle{}, vve::Filename{cube_obj}, aiProcess_FlipWindingOrder
                    });
                    m_engine.SendMessage(MsgPlaySound{ vve::Filename{"assets/sounds/putdown.wav"}, 1, 100 });

                    m_cubeCollected--; 


                    std::cout << "spacebar clicked " << m_cubeCollected << std::endl;
                }
                break;
            }
        }
    

        return false;
    }
    

    bool OnUpdate(Message& message) {
        auto msg = message.GetData<vve::System::MsgUpdate>();
        auto& playerPos = m_registry.Get<vve::Position&>(m_playerHandle)();
        auto& playerRot = m_registry.Get<vve::Rotation&>(m_playerHandle)();
    
        // Check cube pickups
        for (auto it = m_cubeHandles.begin(); it != m_cubeHandles.end(); ) {
            if (!(*it).IsValid()) {
                it = m_cubeHandles.erase(it);
                continue;
            }
    
            auto& cubePos = m_registry.Get<vve::Position&>(*it)();
            float dist = glm::length(glm::vec2(playerPos.x, playerPos.y) - glm::vec2(cubePos.x, cubePos.y));
            if (dist < 1.0f) {
                m_engine.SendMessage(MsgObjectDestroy{ vve::ObjectHandle(*it) });
                it = m_cubeHandles.erase(it);
                m_engine.SendMessage(MsgPlaySound{ vve::Filename{"assets/sounds/pickup.wav"}, 1, 100 });
                m_cubeCollected++;
            } else {
                ++it;
            }
        }
    
        return false;
    }
    

    bool OnRecordNextFrame(Message) {
        ImGui::Begin("Player State: testing testing");
        const char* stateText = (m_playerState == PlayerState::MOVING) ? "MOVING" : "STATIONARY";
        ImGui::Text("State: %s", stateText);
        auto& playerPos = m_registry.Get<vve::Position&>(m_playerHandle)();
        ImGui::Text("player z coordinate: %.2f", playerPos.y);
        ImGui::Text("player x coordinate: %.2f", playerPos.x);
        ImGui::Text("Cubes collected: %d", m_cubeCollected);
        ImGui::End();
        return false;
    }
};

int main() {
    vve::Engine engine("Cypher Game", VK_MAKE_VERSION(1, 3, 0));
    MyGame game{ engine };
    engine.Run();
    return 0;
}
