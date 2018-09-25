#include "GameMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "vertex_color_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>

//TODO: CHANGE
MeshBuffer::Mesh jerry_mesh;
MeshBuffer::Mesh obj_mesh;

Load< MeshBuffer > gameAssets(LoadTagDefault, [](){
	MeshBuffer const *ret = new MeshBuffer(data_path("catAndMouse.pnc"));
	return ret;
});

Load< GLuint > gameAssets_for_vertex_color_program(LoadTagDefault, [](){
	return new GLuint(gameAssets->make_vao_for_program(vertex_color_program->program));
});

/*
Load< MeshBuffer > meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("paddle-ball.pnc"));
});
Load< GLuint > meshes_for_vertex_color_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(vertex_color_program->program));
});
*/

//CHANGED
Scene::Transform *jerry_transform = nullptr;
Scene::Transform *obj_transform = nullptr;
//toms object transform array
std::vector<Scene::Transform *> toms_transforms;
//toms newly dropped objects
std::vector<glm::vec2> newlyDropped;

//Scene::Transform *paddle_transform = nullptr;
//Scene::Transform *ball_transform = nullptr;

Scene::Camera *camera = nullptr;

//CHANGED
Load< Scene > scene(LoadTagDefault, [](){
	Scene *ret = new Scene;
	//load transform hierarchy:
	//ret->load(data_path("paddle-ball.scene"), [](Scene &s, Scene::Transform *t, std::string const &m){
	ret->load(data_path("catAndMouse.scene"), [](Scene &s, Scene::Transform *t, std::string const &m){
		Scene::Object *obj = s.new_object(t);
		obj->program = vertex_color_program->program;
		obj->program_mvp_mat4  = vertex_color_program->object_to_clip_mat4;
		obj->program_mv_mat4x3 = vertex_color_program->object_to_light_mat4x3;
		obj->program_itmv_mat3 = vertex_color_program->normal_to_light_mat3;
		//MeshBuffer::Mesh const &mesh = meshes->lookup(m);
		MeshBuffer::Mesh const &game_asset = gameAssets->lookup(m);
		obj->vao = *gameAssets_for_vertex_color_program;
		obj->start = game_asset.start;
		obj->count = game_asset.count;
		//obj->start = mesh.start;
		//obj->count = mesh.count;
	});

	for (Scene::Transform *t = ret->first_transform; t != nullptr; t = t->alloc_next) {
		std::cout << "object is '" << t->name << "'" <<std::endl;
		if (t->name == "jerry") {
			if (jerry_transform) throw std::runtime_error("Multiple jerry transforms in scene.");
			jerry_transform = t;
		}
		if (t->name == "donut") {
			std::cout << "got donut" << std::endl;
			if (obj_transform) throw std::runtime_error("Multiple obj transforms in scene.");
			obj_transform = t;
		}
	}
	//look up paddle and ball transforms:
	/*
	for (Scene::Transform *t = ret->first_transform; t != nullptr; t = t->alloc_next) {
		if (t->name == "Paddle") {
			if (paddle_transform) throw std::runtime_error("Multiple 'Paddle' transforms in scene.");
			paddle_transform = t;
		}
		if (t->name == "Ball") {
			if (ball_transform) throw std::runtime_error("Multiple 'Ball' transforms in scene.");
			ball_transform = t;
		}
	}
	*/

	if (!jerry_transform) throw std::runtime_error("No jerry transform in scene.");
	if (!obj_transform) throw std::runtime_error("No object transform in scene.");

	//if (!paddle_transform) throw std::runtime_error("No 'Paddle' transform in scene.");
	//if (!ball_transform) throw std::runtime_error("No 'Ball' transform in scene.");

	//look up the camera:
	/*
	{ //Camera looking at the origin:
		Scene::Transform *transform = scene.new_transform();
		transform->position = 
		transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		camera = scene.new_camera(transform);
	}
	*/
	for (Scene::Camera *c = ret->first_camera; c != nullptr; c = c->alloc_next) {
		c->transform->position = glm::vec3(0.0f, -10.0f, 1.0f);
		//Cameras look along -z, so rotate view to look at origin:
		c->transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		if (c->transform->name == "Camera") {
			if (camera) throw std::runtime_error("Multiple 'Camera' objects in scene.");
			camera = c;
		}
	}
	if (!camera) throw std::runtime_error("No 'Camera' camera in scene.");
	
	return ret;
});

GameMode::GameMode(Client &client_) : client(client_) {
	client.connection.send_raw("h", 1); //send a 'hello' to the server
}

GameMode::~GameMode() {
}

glm::vec2 currObject;

//CHANGED
bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

	//move jerry on right/left press
	if (evt.type == SDL_KEYDOWN && evt.key.repeat == 0) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT) { //left arrow pressed
			std::cout<< "jerry tries to move left" <<std::endl;
			if (state.jerry.x-=jerryStep >= Game::FrameWidth) { //bounds checking
				state.jerry.x -= jerryStep;
			}
			return true;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_RIGHT) { //right arrow pressed
			std::cout<< "jerry tries to move right" <<std::endl;
			if (state.jerry.x+=jerryStep <= Game::FrameWidth) { //bounds checking
				state.jerry.x += jerryStep;
			}
			return true;
		}
		//Tom dropping object
		else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
			std::cout<< "tom object drop position x "<< currObject.x << "tom object drop position y " << currObject.y << std::endl;
			newlyDropped.push_back(currObject);
			return true;
		}
	}
	//Tom uses mouse to pick where to drop item and hit d to drop
	if (evt.type == SDL_MOUSEMOTION) {
		//only need x info to drop since it will start from the same y every time (0)
		currObject.y = 0;
		currObject.x = (evt.motion.x - 0.5f * window_size.x) / (0.5f * window_size.x) * Game::FrameWidth;
		currObject.x = std::max(currObject.x, -0.5f * Game::FrameWidth + 0.5f * Game::objectRadius);
		currObject.x = std::min(currObject.x,  0.5f * Game::FrameWidth - 0.5f * Game::objectRadius);
	}

	/*
	if (evt.type == SDL_MOUSEMOTION) {
		state.paddle.x = (evt.motion.x - 0.5f * window_size.x) / (0.5f * window_size.x) * Game::FrameWidth;
		state.paddle.x = std::max(state.paddle.x, -0.5f * Game::FrameWidth + 0.5f * Game::PaddleWidth);
		state.paddle.x = std::min(state.paddle.x,  0.5f * Game::FrameWidth - 0.5f * Game::PaddleWidth);
	}
	*/
	return false;
}

//checks collision between jerry and objects on screen. returns true if jerry is still alive
bool collisionCheck(std::vector<glm::vec2> objectList, glm::vec2 jerryPos,
					float jerryWidth, float jerryHeight, float objectRadius) {
	for (uint32_t i=0; i<objectList.size(); i++) {
		//collision check
		glm::vec2 obj = objectList[i];
		if (jerryPos.x<obj.x+objectRadius and
			jerryPos.x+jerryWidth>obj.x and
			jerryPos.y<obj.y+objectRadius and
			jerryPos.y+jerryHeight>obj.y) {
			return false;
		}
	}
	return true;
}

//TODO: change update so it updates each client
void GameMode::update(float elapsed) {
	//state.update(elapsed);
	//std::cout << "client connection type is " << client.connection.type << std::endl;
	//calculate if jerry has collided with an object on screen right now
	bool jerryLive = collisionCheck(state.onScreenObjList, state.jerry,
								state.jerryWidth, state.jerryHeight, state.objectRadius);

	//send jerry info
	if (client.connection.type == "Jerry") {
		//std::cout<< "jerry sends info" << std::endl;
		//send jerrys state to server:
		client.connection.send_raw("j", 1);
		client.connection.send_raw(&state.jerry.x, sizeof(float));
		//tell server if jerry is still alive
		client.connection.send_raw(&jerryLive, sizeof(bool));
	}
	//send tom info
	if (client.connection.type == "Tom") {
		//std::cout<< "tom sends info" << std::endl;
		//send toms state to server
		client.connection.send_raw("t", 1);
		//tell server to expect this many objects dropped by tom
		uint32_t numObjs = newlyDropped.size();
		client.connection.send_raw(&numObjs, sizeof(uint32_t));
		//client.connection.send_raw(&newlyDropped, numObjs*sizeof(glm::vec2)+sizeof(std::vector));
		for (uint32_t i=0; i<numObjs; i++) {
			client.connection.send_raw(&newlyDropped[i], sizeof(glm::vec2));
		}
		//clear newlyDropped
		newlyDropped = std::vector<glm::vec2>();
		//tell server if tom has lost or not
		bool tomLose = false;
		if (state.objsLeftforTom==0 and state.onScreenObjList.size()==0 and jerryLive) {
			//no objects left for tom and no objects on screen
			tomLose = true;
		}
		client.connection.send_raw(&tomLose, sizeof(bool));
	}

	//receive data from server
	client.poll([&](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			//probably won't get this.
		}
		else if (event == Connection::OnClose) {
			std::cerr << "Lost connection to server." << std::endl;
		}
		//data received from server here
		else { assert(event == Connection::OnRecv);
			//std::cerr << "Ignoring " << c->recv_buffer.size() << " bytes from server." << std::endl;
			if (c->recv_buffer[0] == 's') {
				//std::cout<< "receiving from server" << std::endl;
				//stuff from server, onScreenObjects list (sizeof(uint32_t)*sizeof(glm::vec2)) and
				//jerrys x position (sizeof(float))
				if (c->recv_buffer.size() < 1 + sizeof(uint32_t)) {return;} //wait for more data
				//get size info of what's about to be sent
				uint32_t numOnScreen; 
				memcpy(&numOnScreen, c->recv_buffer.data()+1, sizeof(uint32_t));
				//now that there is more complete info check again
				if (c->recv_buffer.size() < 1+sizeof(uint32_t)+numOnScreen*sizeof(glm::vec2)+sizeof(float)) {
					return; //wait for more data
				}
				//clear Game's onScreenObjList, its about to be overwritten
				state.onScreenObjList = std::vector<glm::vec2>();
				//parse this info, first its tom's current on screen object list
				for (uint32_t j=0; j<numOnScreen; j++) {
					glm::vec2 objPosition;
					memcpy(&objPosition, c->recv_buffer.data()+1+j*sizeof(glm::vec2), sizeof(glm::vec2));
					state.onScreenObjList.push_back(objPosition);
					std::cout<< "dropped object x is "<< objPosition.x << "and y is " << objPosition.y << std::endl;
				}
				//then its jerrys position
				memcpy(&state.jerry.x, c->recv_buffer.data()+1+numOnScreen*sizeof(glm::vec2), sizeof(float));
			}
			if (c->recv_buffer[0] == 'T') { //set tom connection
				client.connection.type = "Tom";
				std::cout << "I am now tom" << std::endl;
			}
			if (c->recv_buffer[0] == 'J') { //set jerry connection
				client.connection.type = "Jerry";
				std::cout << "I am now Jerry" << std::endl;
			}
			c->recv_buffer.clear();
		}
	});

	//CHANGED
	//copy game state to scene positions for drawing updates:
	jerry_transform->position.x = state.jerry.x;
	//copy donut transforms
	/*
	for (uint32_t j=0; j<numOnScreen; j++) {
		Scene::Transform *currObjTrans = scene.new_transform();
		currObjTrans->position = glm::vec3(state.onScreenObjList[j].x, state.onScreenObjList[j].y, 0);
		donut = attach_object(transform1, "donut");
		toms_transforms.push_back(currObjTrans);
	}
	*/
}

//TODO: CHECK EACH OBJECTS VISIBILITY
void GameMode::draw(glm::uvec2 const &drawable_size) {
	camera->aspect = drawable_size.x / float(drawable_size.y);

	glClearColor(0.25f, 0.0f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//set up light positions:
	glUseProgram(vertex_color_program->program);

	glUniform3fv(vertex_color_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.81f, 0.81f, 0.76f)));
	glUniform3fv(vertex_color_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
	glUniform3fv(vertex_color_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
	glUniform3fv(vertex_color_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));

	scene->draw(camera);

	GL_ERRORS();
}
