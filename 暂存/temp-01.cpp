//In the normal version, please remove this:
typedef double Math_F;
#include <cmath>
//

#include <iostream> 
using namespace std;

template<typename Ty>
class Value
{
//private:
public:
	int mode;//0: real value;1
	Ty rv;
	const Value *a=nullptr,*b=nullptr;
	enum class Opt
	{
		add,sub,mlt,div,pow
	}opt;
	
public:
	Value(Ty x=0)
	{
		mode=0;
		rv=x;
	}
	
	//Work, then return the result
	Ty get() const
	{
		if(mode==0)return rv;
		else
		{
			switch(opt)
			{
				case Opt::add:return a->get()+b->get();break;
				case Opt::sub:return a->get()-b->get();break;
				case Opt::mlt:return a->get()*b->get();break;
				case Opt::div:return a->get()/b->get();break;
				case Opt::pow:return pow(a->get(),b->get());break;
			}
		}
	}
	
	//Set the value
	bool set(Ty x)
	{
		if(mode==0)
		{
			rv=x;
			return true;
		}
		else return false;
	}

	//Operations
	Value operator+(const Value &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::add;
		return res;
	}
};

typedef Value<Math_F> Value_F;

int main()
{
	Value_F a=1, b=2;
	auto c=a+b;
	cout<<c.get()<<endl;
}
