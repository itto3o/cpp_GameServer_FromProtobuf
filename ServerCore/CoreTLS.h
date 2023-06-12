#pragma once
#include <stack>

extern thread_local uint32				LThreadId;
extern thread_local uint64				LEndTickCount;
extern thread_local std::stack<int32>	LLockStack;
extern thread_local SendBufferChunkRef	LSendBufferChunk;

// 뭘 실행하는지 알기위한 용도라서 굳이 스마트포인터까지 갈필요없이 그냥 포인터로만, 그냥주소값만 들고 있으면 됨
extern thread_local class JobQueue*		LCurrentJobQueue;