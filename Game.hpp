#pragma once

#include <glm/glm.hpp>
#include <random>

//CHANGED
struct Game {
    glm::vec2 jerry = glm::vec2(0.0f,-3.0f);
    //tom's objects that have already been dropped. Tom and Jerry draw from this list.
    std::vector<glm::vec2> onScreenObjList;
    glm::vec2 objectSpeed = glm::vec2(0.0f,-4.0f); //no x horiz speed just y speed
    
    //srand(time(NULL));
    //uint32_t objsLeftforTom = rand()%20; //get between 0 and 20 objects to throw
    uint32_t objsLeftforTom = 20;
    
    void update(float time);

    static constexpr const float FrameWidth = 10.0f;
    static constexpr const float FrameHeight = 8.0f;
    static constexpr const float jerryWidth = 2.0f;
    static constexpr const float jerryHeight = 0.4f;
    static constexpr const float objectRadius = 0.5f;
};
/*
struct Game {
	glm::vec2 paddle = glm::vec2(0.0f,-3.0f);
	glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball_velocity = glm::vec2(0.0f,-2.0f);

	void update(float time);

	static constexpr const float FrameWidth = 10.0f;
	static constexpr const float FrameHeight = 8.0f;
	static constexpr const float PaddleWidth = 2.0f;
	static constexpr const float PaddleHeight = 0.4f;
	static constexpr const float BallRadius = 0.5f;
};
*/
