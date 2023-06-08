#pragma once

// C++11 버전의 apply를 직접 만들어보기
// 해석
// tuple : 여러 데이터를 모아놓는 애 pair와 비슷하게,
// 데이터를 꺼내쓰고 싶다면 std::get<0>(tup)으로 0번째 데이터를 꺼내쓸 수 있음
// 1번은 get<1>(tup)
//
template<int... Remains>
struct seq
{};

// gen_seq는 gen_seq를 상속받고 있음
// gen_seq<3>을 만들면, gen_seq<2, 2>를 상속받고 있음
// ==> gen<N=2, remains.. = 2> -> gen<1,1,2> 를 상속받게 됨
// ==> gen<0, 0, 1, 2>가 만들어짐,
// 앞이 0이 되면 0은 이 밑의 버전으로 인지를 해서 밑에있는 애가 호출됨
template<int N, int... Remains>
struct gen_seq : gen_seq<N - 1, N - 1, Remains...>
{};

// gen_seq가 0으로시작하면 seq에 remains를 넘긴 애를 상속받게 됨
// : seq<0, 1, 2>
// 즉, gen_seq<3>은 seq<0,1,2>를 상속받게 됨
template<int... Remains>
struct gen_seq<0, Remains...> : seq<Remains...> // seq 상속받음
{};

template<typename Ret, typename... Args>
void xapply(Ret(*func)(Args...), std::tuple<Args...>& tup)
{
										// sizeof는 args의 개수를 반환하게 됨, 
										//밑에서 tuple을 인자로 넎어주니까 tuple으 ㅣ개수,
										// FuncJob<void, int64, int32>로 넣으면 gen_seq<2>가 됨
	//return 
	// return 타입이 없으니까 지금은 지워주고, 있다면 helper에서 return 해주면 됨
	xapply_helper(func, gen_seq<sizeof...(Args)>(), tup);
}

						// gen_seq로 만들어진 seq를 받아줌, ls를 풀어씀, 그 모
template<typename F, typename... Args, int... ls> //ls : list 약자
void xapply_helper(F func, seq<ls...>, std::tuple<Args...>& tup)
{
	// 함수를 호출하는데, 인자들을 만들어주고 있음
	// std::get<0>(tup), std::get<1>(tup), std::get<2>(tup) 모든 인자들을 하나씩 꺼내서 func에 넘겨주고있음
	// seq는 뭘하길래?
	// ==> get_seq<3>(); 으로 만든다면 하나의 구조체임, 인자가 3인
	(func)(std::get<ls>(tup)...);
}


// 멤버함수 버전
template<typename T, typename Ret, typename... Args>
void xapply(T* obj, Ret(T::*func)(Args...), std::tuple<Args...>& tup)
{
	// sizeof는 args의 개수를 반환하게 됨, 
	//밑에서 tuple을 인자로 넎어주니까 tuple으 ㅣ개수,
	// FuncJob<void, int64, int32>로 넣으면 gen_seq<2>가 됨
	xapply_helper(obj, func, gen_seq<sizeof...(Args)>(), tup);
}

// gen_seq로 만들어진 seq를 받아줌, ls를 풀어씀, 그 모
template<typename T, typename F, typename... Args, int... ls> //ls : list 약자
void xapply_helper(T* obj, F func, seq<ls...>, std::tuple<Args...>& tup)
{
	// 함수를 호출하는데, 인자들을 만들어주고 있음
	// std::get<0>(tup), std::get<1>(tup), std::get<2>(tup) 모든 인자들을 하나씩 꺼내서 func에 넘겨주고있음
	// seq는 뭘하길래?
	// ==> get_seq<3>(); 으로 만든다면 하나의 구조체임, 인자가 3인
	(obj->*func)(std::get<ls>(tup)...);
}

class IJob
{
public:
	virtual void Execute() { }
};

// 함수자(Functor) 어떤 객체인데 그 객체가 사실상 함수 역할을 함
// 사용해야되는 함수가 있고, target, value를 인자로 받아서 cout 로그를 찍어주는 함수라면
// job으로 만드는게 오늘의 목표, 함수자에 만들어주기,
// 함수 자체도 Job에, 인자도 같이 Job에 넘겨줘야
// 근데 인자도 같이 넘겨줘야하는데 template으로 하면 추가적으로 늘려서 받아줄 수 있음
// 하지만 인자가 몇개인지 알 수 없음, 0개~여러개 ㄱㄴ
// template문법에선 동일한 class이름으로 오버로딩?같은 개념은 안됨 ==> ...을 쓰기(가변적임)
template<typename Ret, typename... Args>
class FuncJob : public IJob
{
	// 반환타입은 Ret이고 인자를 Args만큼 받아주는 funcType으로 선언
	// but, 모든 함수 타입을 받아줄 수 있는 건 아님
	// 함수는 여러종류가 있음,
	// Knight class안의 HealMe라는 멤버 함수가 있다면, 함수 호출 규악이 다름
	// --> 
	using FuncType = Ret(*)(Args...); // Ret(*)가 함수, Ret(*func)으로하면 func이 함수 이름이 됨
public:
	FuncJob(FuncType func, Args... args) : _func(func), _tuple(args...)
	{

	}
	// Ret은 받아온 값, 같이넘겨준 값
	// () operator를 쓸 수 있어서 job(); 이런 식으로 사용 가능
	//Ret operator()(Args... args) <- tuple로 인자들을 내부에 저장하고 있어서 이런식으로는 호출x
	//Ret operator()()
	//{
		// 여기에 직접적인 코드가 들어가는게 아니라
		// 이미 만들어진 함수가 실행되도록 연결해주고 싶은 것(gameserver의 HealByValue)
		// ==> 함수 포인터 사용
		//_func(args...);
		//std::apply(_func, _tuple); //c++17 기준으로 tuple, func을 바로 꽂아줄 수 있음
		// 컴파일러가 C++ 17로 안만들어졌다면 에러가 남
		// ==> 프로젝트 우클릭>속성>C/C++>언어>기본값으로 되어있는 애를 C++17표준으로 바꿔주기
		// 그럼 예전엔 어떻게 했을까?
		// C++ 11으로 맞춰서하는 프로젝트도 많은데 
	//}

	// 위와 같은 의미의 함수
	//Ret Execute()
	virtual void Execute() override
	{
		//_func(args...);
		//std::apply(_func, _tuple);
		xapply(_func, _tuple);
	}

private:
	FuncType _func; // 멤버변수로 함수를 들고있기
	std::tuple<Args...> _tuple; //args를 튜플로 들고있기
};
		// 객체(멤버함수를 갖고있는) 의 타입을 T로 받아주기
template<typename T, typename Ret, typename... Args>
class MemberJob : public IJob
{
	using FuncType = Ret(T::*)(Args...); // T에 포함된 멤버함수인 것을 표시
public:
	MemberJob(T* obj, FuncType func, Args... args) : _obj(obj), _func(func), _tuple(args...)
	{

	}

	//Ret Execute()
	virtual void Execute() override
	{
		xapply(_obj, _func, _tuple);
	}

private:
	T*						_obj;
	FuncType				_func; 
	std::tuple<Args...>		_tuple; 
};


///	//////////////////

// 인터페이스 만들기
// 얘를 이후시간에 배운 애로 고쳐볼건데, 얘를 맨 위로 올려야함
//class IJob
//{
//public:
//	virtual void Execute() { }
//};

// 위의 인터페이스를 상속을 받아서 일을 만들것
//class HealJob : public IJob
//{
//public:
//	virtual void Execute() override
//	{
//		// _target을 찾아서
//		// _target0->AddHP(_healValue);
//		cout << _target << "한테 힐" << _healValue << " 만큼 줌";
//	}
//public:
//	// heal을 1번 유저한테 줘야하니까
//	// 인자들을 같이 들고 있어야함
//	uint64 _target = 0;
//	uint32 _healValue = 0;
//};

// jobQueue로 순차적 처리할 수 있게끔
using JobRef = shared_ptr<IJob>;

class JobQueue
{
public:
	void Push(JobRef job)
	{
		WRITE_LOCK;
		_jobs.push(job);
	}

	JobRef Pop()
	{
		WRITE_LOCK;
		if (_jobs.empty())
			return nullptr;

		JobRef ret = _jobs.front();
		_jobs.pop();
		return ret;

	}

private:
	USE_LOCK;
	queue<JobRef> _jobs;
};
