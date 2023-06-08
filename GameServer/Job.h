#pragma once

// C++11 ������ apply�� ���� ������
// �ؼ�
// tuple : ���� �����͸� ��Ƴ��� �� pair�� ����ϰ�,
// �����͸� �������� �ʹٸ� std::get<0>(tup)���� 0��° �����͸� ������ �� ����
// 1���� get<1>(tup)
//
template<int... Remains>
struct seq
{};

// gen_seq�� gen_seq�� ��ӹް� ����
// gen_seq<3>�� �����, gen_seq<2, 2>�� ��ӹް� ����
// ==> gen<N=2, remains.. = 2> -> gen<1,1,2> �� ��ӹް� ��
// ==> gen<0, 0, 1, 2>�� �������,
// ���� 0�� �Ǹ� 0�� �� ���� �������� ������ �ؼ� �ؿ��ִ� �ְ� ȣ���
template<int N, int... Remains>
struct gen_seq : gen_seq<N - 1, N - 1, Remains...>
{};

// gen_seq�� 0���ν����ϸ� seq�� remains�� �ѱ� �ָ� ��ӹް� ��
// : seq<0, 1, 2>
// ��, gen_seq<3>�� seq<0,1,2>�� ��ӹް� ��
template<int... Remains>
struct gen_seq<0, Remains...> : seq<Remains...> // seq ��ӹ���
{};

template<typename Ret, typename... Args>
void xapply(Ret(*func)(Args...), std::tuple<Args...>& tup)
{
										// sizeof�� args�� ������ ��ȯ�ϰ� ��, 
										//�ؿ��� tuple�� ���ڷ� �����ִϱ� tuple�� �Ӱ���,
										// FuncJob<void, int64, int32>�� ������ gen_seq<2>�� ��
	//return 
	// return Ÿ���� �����ϱ� ������ �����ְ�, �ִٸ� helper���� return ���ָ� ��
	xapply_helper(func, gen_seq<sizeof...(Args)>(), tup);
}

						// gen_seq�� ������� seq�� �޾���, ls�� Ǯ�, �� ��
template<typename F, typename... Args, int... ls> //ls : list ����
void xapply_helper(F func, seq<ls...>, std::tuple<Args...>& tup)
{
	// �Լ��� ȣ���ϴµ�, ���ڵ��� ������ְ� ����
	// std::get<0>(tup), std::get<1>(tup), std::get<2>(tup) ��� ���ڵ��� �ϳ��� ������ func�� �Ѱ��ְ�����
	// seq�� ���ϱ淡?
	// ==> get_seq<3>(); ���� ����ٸ� �ϳ��� ����ü��, ���ڰ� 3��
	(func)(std::get<ls>(tup)...);
}


// ����Լ� ����
template<typename T, typename Ret, typename... Args>
void xapply(T* obj, Ret(T::*func)(Args...), std::tuple<Args...>& tup)
{
	// sizeof�� args�� ������ ��ȯ�ϰ� ��, 
	//�ؿ��� tuple�� ���ڷ� �����ִϱ� tuple�� �Ӱ���,
	// FuncJob<void, int64, int32>�� ������ gen_seq<2>�� ��
	xapply_helper(obj, func, gen_seq<sizeof...(Args)>(), tup);
}

// gen_seq�� ������� seq�� �޾���, ls�� Ǯ�, �� ��
template<typename T, typename F, typename... Args, int... ls> //ls : list ����
void xapply_helper(T* obj, F func, seq<ls...>, std::tuple<Args...>& tup)
{
	// �Լ��� ȣ���ϴµ�, ���ڵ��� ������ְ� ����
	// std::get<0>(tup), std::get<1>(tup), std::get<2>(tup) ��� ���ڵ��� �ϳ��� ������ func�� �Ѱ��ְ�����
	// seq�� ���ϱ淡?
	// ==> get_seq<3>(); ���� ����ٸ� �ϳ��� ����ü��, ���ڰ� 3��
	(obj->*func)(std::get<ls>(tup)...);
}

class IJob
{
public:
	virtual void Execute() { }
};

// �Լ���(Functor) � ��ü�ε� �� ��ü�� ��ǻ� �Լ� ������ ��
// ����ؾߵǴ� �Լ��� �ְ�, target, value�� ���ڷ� �޾Ƽ� cout �α׸� ����ִ� �Լ����
// job���� ����°� ������ ��ǥ, �Լ��ڿ� ������ֱ�,
// �Լ� ��ü�� Job��, ���ڵ� ���� Job�� �Ѱ����
// �ٵ� ���ڵ� ���� �Ѱ�����ϴµ� template���� �ϸ� �߰������� �÷��� �޾��� �� ����
// ������ ���ڰ� ����� �� �� ����, 0��~������ ����
// template�������� ������ class�̸����� �����ε�?���� ������ �ȵ� ==> ...�� ����(��������)
template<typename Ret, typename... Args>
class FuncJob : public IJob
{
	// ��ȯŸ���� Ret�̰� ���ڸ� Args��ŭ �޾��ִ� funcType���� ����
	// but, ��� �Լ� Ÿ���� �޾��� �� �ִ� �� �ƴ�
	// �Լ��� ���������� ����,
	// Knight class���� HealMe��� ��� �Լ��� �ִٸ�, �Լ� ȣ�� �Ծ��� �ٸ�
	// --> 
	using FuncType = Ret(*)(Args...); // Ret(*)�� �Լ�, Ret(*func)�����ϸ� func�� �Լ� �̸��� ��
public:
	FuncJob(FuncType func, Args... args) : _func(func), _tuple(args...)
	{

	}
	// Ret�� �޾ƿ� ��, ���̳Ѱ��� ��
	// () operator�� �� �� �־ job(); �̷� ������ ��� ����
	//Ret operator()(Args... args) <- tuple�� ���ڵ��� ���ο� �����ϰ� �־ �̷������δ� ȣ��x
	//Ret operator()()
	//{
		// ���⿡ �������� �ڵ尡 ���°� �ƴ϶�
		// �̹� ������� �Լ��� ����ǵ��� �������ְ� ���� ��(gameserver�� HealByValue)
		// ==> �Լ� ������ ���
		//_func(args...);
		//std::apply(_func, _tuple); //c++17 �������� tuple, func�� �ٷ� �Ⱦ��� �� ����
		// �����Ϸ��� C++ 17�� �ȸ�������ٸ� ������ ��
		// ==> ������Ʈ ��Ŭ��>�Ӽ�>C/C++>���>�⺻������ �Ǿ��ִ� �ָ� C++17ǥ������ �ٲ��ֱ�
		// �׷� ������ ��� ������?
		// C++ 11���� ���缭�ϴ� ������Ʈ�� ������ 
	//}

	// ���� ���� �ǹ��� �Լ�
	//Ret Execute()
	virtual void Execute() override
	{
		//_func(args...);
		//std::apply(_func, _tuple);
		xapply(_func, _tuple);
	}

private:
	FuncType _func; // ��������� �Լ��� ����ֱ�
	std::tuple<Args...> _tuple; //args�� Ʃ�÷� ����ֱ�
};
		// ��ü(����Լ��� �����ִ�) �� Ÿ���� T�� �޾��ֱ�
template<typename T, typename Ret, typename... Args>
class MemberJob : public IJob
{
	using FuncType = Ret(T::*)(Args...); // T�� ���Ե� ����Լ��� ���� ǥ��
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

// �������̽� �����
// �긦 ���Ľð��� ��� �ַ� ���ĺ��ǵ�, �긦 �� ���� �÷�����
//class IJob
//{
//public:
//	virtual void Execute() { }
//};

// ���� �������̽��� ����� �޾Ƽ� ���� �����
//class HealJob : public IJob
//{
//public:
//	virtual void Execute() override
//	{
//		// _target�� ã�Ƽ�
//		// _target0->AddHP(_healValue);
//		cout << _target << "���� ��" << _healValue << " ��ŭ ��";
//	}
//public:
//	// heal�� 1�� �������� ����ϴϱ�
//	// ���ڵ��� ���� ��� �־����
//	uint64 _target = 0;
//	uint32 _healValue = 0;
//};

// jobQueue�� ������ ó���� �� �ְԲ�
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
