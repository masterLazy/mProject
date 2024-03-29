//In the normal version, please remove this:
typedef double Math_F;
#include <cmath>
#define M_E 2.718281828
//

#include <iostream> 
using namespace std;

template<typename Ty>
class Value
{
private:
	int mode;//0: real value;1
	Ty rv;
	const Value *a=nullptr,*b=nullptr;
	enum class Opt
	{
		add,sub,mlt,div,pow,log
	}opt;
	
public:
	Value(Ty x=0)
	{
		mode=0;
		rv=x;
	}
		
	template<typename Ty2>
	Value(Ty2 x=0)
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
				case Opt::add:return a->get()+b->get();
				case Opt::sub:return a->get()-b->get();
				case Opt::mlt:return a->get()*b->get();
				case Opt::div:return a->get()/b->get();
				case Opt::pow:return ::pow(a->get(),b->get());
				case Opt::log:return ::log(a->get())/::log(b->get());
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

	//Derivation
	//With something wrong!
	Value<Ty> dvt(Value<Ty> &x) const
	{
		//Itself 
		if(this==&x)return 1;
		//Constant value
		else if(mode==0)return 0;
		
		switch(opt)
		{
			case Opt::add:return a->dvt(x)+b->dvt(x);
			case Opt::sub:return a->dvt(x)-b->dvt(x);
			//[f(x)g(x)]'=f'(x)g(x)+f(x)g'(x)
			case Opt::mlt:return a->dvt(x)*(*b)+(*a)*b->dvt(x);
			//[f(x)/g(x)]'=[f'(x)g(x)-f(x)g'(x)]/[g(x)]^2
			case Opt::div:return (a->dvt(x)*(*b)-(*a)*b->dvt(x))/((*b)*(*b));
			//[f(x)^g(x)]'=f(x)^g(x)*{g'(x)*ln[f(x)]+g(x)/f(x)*f'(x)}
			case Opt::pow:return a->pow(*b)*(b->dvt(x)*a->ln()+(*b)/(*a)*a->dvt(x));
		}
	}

	//Operator
	Value<Ty> operator+(const Value<Ty> &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::add;
		return res;
	}
	Value<Ty> operator-(const Value<Ty> &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::sub;
		return res;
	}
	Value<Ty> operator*(const Value<Ty> &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::mlt;
		return res;
	}
	Value<Ty> operator/(const Value<Ty> &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::div;
		return res;
	}
	Value<Ty> operator-() const
	{
		Value res;
		static Value zero=0;
		res.mode=1;
		res.a=&zero;
		res.b=&b;
		res.opt=Opt::sub;
		return res;
	}
	Ty operator=(Ty b)
	{
		set(b);
		return b;
	}
	
	//Other operator
	//1/x
	Value<Ty> rec() const
	{
		Value res;
		static Value one=1;
		res.mode=1;
		res.a=&one;
		res.b=&b;
		res.opt=Opt::div;
		return res;
	} 
	//a^b
	Value<Ty> pow(const Value<Ty> &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::pow;
		return res;
	}
	//log_b(a)
	Value<Ty> log(const Value<Ty> &b) const
	{
		Value res;
		res.mode=1;
		res.a=this;
		res.b=&b;
		res.opt=Opt::log;
		return res;
	}
	Value<Ty> ln() const
	{
		return log(M_E);
	}
	
	//friend-
	//With something wrong!
	Value<Ty> friend operator+(Ty a, const Value<Ty> &b)
	{
		return b+a;
	}
	Value<Ty> friend operator-(Ty a,const Value<Ty> &b)
	{
		return -b+a;
	}
	Value<Ty> friend operator*(Ty a,const Value<Ty> &b)
	{
		return b*a;
	}
	Value<Ty> friend operator/(Ty a,const Value<Ty> &b)
	{
		return b.rec()*a;
	}
	
	//Compare
	bool operator==(const Value<Ty> &b) const
	{
		return get()==b.get();
	}
	bool operator!=(const Value<Ty> &b) const
	{
		return get()!=b.get();
	}
	bool operator<(const Value<Ty> &b) const
	{
		return get()<b.get();
	}
	bool operator>(const Value<Ty> &b) const
	{
		return get()>b.get();
	}
	bool operator<=(const Value<Ty> &b) const
	{
		return get()<=b.get();
	}
	bool operator>=(const Value<Ty> &b) const
	{
		return get()>=b.get();
	}
};
template<typename Ty>
Value<Ty> pow(const Value<Ty> &a,const Value<Ty> &b)
{
	return a.pow(b);
}

typedef Value<Math_F> ValueF;

int main()
{
	ValueF x,y,d;
	y=x*2;
	d=y.dvt(x);
	
	for(int i=-2;i<=2;i++)
	{
		x=i;
		cout<<"x="<<i<<",\ty="<<y.get()<<",\tdy/dx="<<d.get()<<endl;
	}
	
	return 0;
}
