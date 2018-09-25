#pragma once

#include "Mode.hpp"

#include "MeshBuffer.hpp"
#include "GL.hpp"
#include "Connection.hpp"
#include "Game.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

// The 'GameMode' mode is the main gameplay mode:

struct GameMode : public Mode {
	GameMode(Client &client);
	virtual ~GameMode();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

	//update is called at the start of a new frame, after events are handled:
	virtual void update(float elapsed) override;

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//------- variables -------
	float jerryStep = 3.0f;

	//CHANGED
	//toms object transform array
	//std::vector<Scene::Transform *> toms_transforms;
	//toms newly dropped objects
	//float currJerryX = 0.0f;
	//float currJerryY = -3.0f;
	std::vector<glm::vec2> newlyDropped;

	//------- game state -------
	Game state;

	//------ networking ------
	Client &client; //client object; manages connection to server.
};
