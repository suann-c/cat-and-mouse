#include "Connection.hpp"
#include "Game.hpp"

#include <iostream>
#include <set>
#include <chrono>

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}
	
	Server server(argv[1]);

	Game state;
	//CHANGED
	uint32_t numPlayers = 0;
	//maintain server side copy of all dropped objects, only deleting them when they hit jerry or the ground
	std::vector<glm::vec2> fallingObjs;
	//std::unordered_map< Connection *, string playerType > players;
	float fallSpeed = -4.0f;
	bool jerryLives;
	bool tomLoses;
	//keep track of initialization
	bool justOpenedTom = false;
	bool justOpenedJerry = false;

	while (1) {
		server.poll([&](Connection *c, Connection::Event evt){
			if (evt == Connection::OnOpen) {
				//CHANGED
				if (numPlayers < 2) {
					numPlayers++;
					if (numPlayers == 1) { //tom has entered
						std::cout << "Tom has entered the game" << std::endl;
						c->type = "Tom";
						justOpenedTom = true;
					}
					else {
						std::cout << "Jerry has entered the game" << std::endl;
						c->type = "Jerry";
						justOpenedJerry = true;
					}
				}
				else { //numPlayers > 2, tell them to wait
					std::cout << "Only one Tom and one Jerry can play at one time." << std::endl;
					c->close();
				}
			} 
			else if (evt == Connection::OnClose) {
				//CHANGED
				//someone has exited the game
				if (numPlayers > 0){ //if it is zero then no one in server
					numPlayers--;
				}
			}
			//CHANGED
			else { assert(evt == Connection::OnRecv);
				//have the server serve 2 clients at once (one tom and one jerry)
				if (c->recv_buffer[0] == 'h') {
					c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1);
					if (numPlayers==1) {
						std::cout << c << ": Got Tom's hello. Waiting for Jerry" << std::endl;
					}
					else if (numPlayers==2) {
						std::cout << c << ": Got Jerry's hello. Starting game..." << std::endl;
					}
				}
				if (c->recv_buffer[0] == 'j') {
					//std::cout << "receiving info from jerry" << std::endl;
					//assert(c->type == "Jerry"); //if not receiving jerry info something is wrong
					if (c->recv_buffer.size() < 1 + sizeof(float)+sizeof(bool)) {
						return; //wait for more data
					}
					else {
						//memcpy jerry's movedata
						memcpy(&state.jerry.x, c->recv_buffer.data() + 1, sizeof(float)); //??
						memcpy(&jerryLives, c->recv_buffer.data()+1+sizeof(float), sizeof(bool));
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin()+1+sizeof(float)+sizeof(bool));
					}
				}
				//receiving toms data for processing
				if (c->recv_buffer[0] == 't') {
					//std::cout << "receiving info from tom" << std::endl;
					//assert(c->type == "Tom"); //if not receiving tom info something is wrong
					if (c->recv_buffer.size() < 1 + sizeof(uint32_t)) {return;} //wait for more data
					uint32_t numDropped; 
					memcpy(&numDropped, c->recv_buffer.data()+1, sizeof(uint32_t));
					//now that data is complete check again
					if (c->recv_buffer.size() < 1 +sizeof(uint32_t)+numDropped*sizeof(glm::vec2)+sizeof(bool)) {
						return; //wait for more data
					}
					else {
						//memcpy toms' new dropped objects list
						for (uint32_t i=0; i<numDropped; i++) {
							glm::vec2 objPosition;
							memcpy(&objPosition, c->recv_buffer.data()+1+i*sizeof(glm::vec2), sizeof(glm::vec2));
							fallingObjs.push_back(objPosition);
						}
						//record tom win/loss condition
						memcpy(&tomLoses, c->recv_buffer.data()+1+numDropped*sizeof(glm::vec2), sizeof(bool));
						//clear buffer of toms data
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin()+1+sizeof(bool)+numDropped*sizeof(glm::vec2));
					}
				}
			}

			//do state calculations here
			//update the y coordinates of all objects on screen and remove those that have hit bottom or jerry
			for (uint32_t j=0; j<fallingObjs.size(); j++) {
				fallingObjs[j].y += fallSpeed; //update each current object in list with fallSpeed
				std::cout<< "tom object drop position x "<< fallingObjs[j].x << "tom object drop position y " << fallingObjs[j].y << std::endl;
				if (fabs(fallingObjs[j].y) >= state.FrameHeight) { //hit ground, remove from list
					fallingObjs.erase(fallingObjs.begin()+j);
				}
			}
			//if (!jerryLives) {} //tom wins the game
			//else {} //jerry wins the game

			//send info out to clients here
			//look through list of connects to find tom and jerry
			for (auto connec = server.connections.begin(); connec!=server.connections.end(); connec++){
				if (connec->type == "Tom") {
					if (justOpenedTom == true) {
						std::cout << "sending t to tom" << std::endl;
						connec->send_raw("T", 1);
						justOpenedTom = false; //just send it once
					}
					else {
						connec->send_raw("s", 1);
						//tell clients to expect this many on screen objects
						uint32_t numFalling = fallingObjs.size();
						connec->send_raw(&numFalling, sizeof(uint32_t));
						for (uint32_t j=0; j<numFalling; j++) {
							connec->send_raw(&fallingObjs[j], sizeof(glm::vec2));
						}
						//send jerrys information
						connec->send_raw(&state.jerry.x, sizeof(float)); //??
					}
				}
				if (connec->type == "Jerry") {
					if (justOpenedJerry == true) {
						std::cout << "sending j to jerry" << std::endl;
						connec->send_raw("J", 1);
						justOpenedJerry = false; //just send it once
					}
					else {
						connec->send_raw("s", 1);
						//tell clients to expect this many on screen objects
						uint32_t numFalling = fallingObjs.size();
						connec->send_raw(&numFalling, sizeof(uint32_t));
						for (uint32_t j=0; j<numFalling; j++) {
							connec->send_raw(&fallingObjs[j], sizeof(glm::vec2));
						}
						//send jerrys information
						connec->send_raw(&state.jerry.x, sizeof(float)); //??
					}
				}
			}

		}, 0.01);

    	/*
    	//send info out to clients here
    	for (auto it = server.connections.begin(); it!=server.connections.end(); it++){
   			if (it->type == "Tom") {
   				;
   			}
   			else if (it->type == "Jerry") {
   				;
   			}
		}
    	*/
		/*
		//every second or so, dump the current jerry position:
		static auto then = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();
		if (now > then + std::chrono::seconds(1)) {
			then = now;
			std::cout << "Current Jerry position: " << state.jerry.x << std::endl;
		}
		*/
	}
}
