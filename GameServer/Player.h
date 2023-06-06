#pragma once
class Player
{
public:
	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	GameSessionRef			ownerSession; // Cycle 발생, 메모리 누수방지를 위해 언젠가 끊어줘야
	// 실행 중에 더미클라를 끈다면 메모리 릭 발생
};

