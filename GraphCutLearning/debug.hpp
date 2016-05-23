#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#ifdef _DEBUG_BUILD_

#define DEBUG(x) x

#include <windows.h>
#include <iostream>
#include <iomanip>

using namespace std;

inline void dout()
{}

template<typename Arg1, typename ...Args>
void dout(Arg1 arg1, Args... args)
{
	cout<<arg1;
	dout(args...);
}

enum ConsoleColor
{
        Black         = 0,
        Blue          = 1,
        Green         = 2,
        Cyan          = 3,
        Red           = 4,
        Magenta       = 5,
        Brown         = 6,
        LightGray     = 7,
		DarkGray      = 8,
        LightBlue     = 9,
        LightGreen    = 10,
        LightCyan     = 11,
        LightRed      = 12,
        LightMagenta  = 13,
        Yellow        = 14,
        White         = 15
};

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

void SetColor(ConsoleColor text, ConsoleColor background)
{
    SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

struct NodG
{
	void * nb; // array of neighbors
	int tag; // name / tag of node. WARNING: do NOT use 0, -1, -2
	unsigned int place; // place in node array
};

struct Nod
{
	const NodG * par;
	double maxflow;
	unsigned int parlink;
	int tree;
	int origin;
	bool active;
	/// + 24 align
};

const int rn = 64;
const int cn = 64;

class ProfTimer {
    LARGE_INTEGER mTimeStart,
				  mTimeChecked,
				  Frequency;
    float TimeElapsed;
public:
	ProfTimer() {QueryPerformanceFrequency(&Frequency);}

    void start() {QueryPerformanceCounter(&mTimeStart);}
    float check(){
        QueryPerformanceCounter(&mTimeChecked);
        return TimeElapsed=(float)(mTimeChecked.QuadPart-mTimeStart.QuadPart)/(float)Frequency.QuadPart;
    }
    float getDur() const {return TimeElapsed;}
};

void showm(const int *m)
{
	for( int i = 0; i < rn; ++i )
	{
		for( int j = 0; j < cn; ++j )
		{
			char x;
			if ( m[i*cn+j+2] == -1 )
				x = '*';
			else if ( m[i*cn+j+2] == -2 )
				x = '.';
			else
				x = ' ';
			cout<<x;
		}
		cout<<"\n";
	}
}

void show(const double * const f, const Nod * const p)
{
	cout<<"___________________\n";
	//cout<<fixed;
	for ( int i = 0; i < rn*2-1; ++i)
	{
		if ( i % 2 == 0 )
		{
			cout<<" ";
		}
		for ( int j = 0; j < cn - (i%2==0); ++j)
		{
			if ( i % 2 == 0 )
			{
				const Nod & q = p[ (i/2)*cn + j + 2 ];
				if ( q.tree == 0 )
					SetColor(Green, Green);
				else if ( q.tree == -1 )
					{
						if ( q.origin )
						{
							if ( q.par && q.par->tag == q.tree )
								SetColor(Cyan, Red);
							else
								SetColor(White, Red);
						}
						else
							SetColor(White, LightRed);
					}
				else if ( q.tree == -2 )
					{
						if ( q.origin )
						{
							if ( q.par && q.par->tag == q.tree )
								SetColor(Cyan, Blue);
							else
								SetColor(White, Blue);
						}
						else
							SetColor(White, LightBlue);
					}
				else
					SetColor(Black, Black);
				cout<<"P";
			}
			SetColor(Black, Black);

			double x = /*f[rn*cn*2 + ( (i%2==0) ? ((i/2)*(cn-1) + j + (rn-1)*cn) : ((i/2)*cn + j) ) ];*/f[rn*cn*2 + ( (i%2==0) ? ((i/2)*(cn-1) + j) : ((i/2)*cn + j + (cn-1)*rn) ) ];
			if ( i % 2 == 1 )
				cout<<"0";

			if ( x <= 0.0000000001 )
				SetColor(Black, Black);
			else
				SetColor(White, White);

			cout/*<<setprecision(1)<<setw((i%2==0)?1:3)*/<<"0";//x;

			SetColor(Black, Black);

			/*if ( i % 2 == 1 )
				cout<<"0";*/
		}
		if ( i % 2 == 0 )
		{
			const Nod & q = p[ (i/2)*cn + (cn-1) + 2 ];
			if ( q.tree == 0 )
				SetColor(Green, Green);
			else if ( q.tree == -1 )
				{
					if ( q.origin )
					{
						if ( q.par && q.par->tag == q.tree )
							SetColor(Cyan, Red);
						else
							SetColor(White, Red);
					}
					else
						SetColor(White, LightRed);
				}
			else if ( q.tree == -2 )
				{
					if ( q.origin )
					{
						if ( q.par && q.par->tag == q.tree )
							SetColor(Cyan, Blue);
						else
							SetColor(White, Blue);
					}
					else
						SetColor(White, LightBlue);
				}
			else
				SetColor(Black, Black);
			cout<<"P";
			SetColor(White, Black);
		}
		cout<<"\n";
		SetColor(White, Black);
	}
	//cout.unsetf(ios_base::fixed);
	/*cout<<"-------------------\n";
	for ( int i = 0; i < rn; ++i)
	{
		for ( int j = 0; j < cn; ++j)
		{
			cout<<setw(3);
			double x = f[i * cn + j];
			if ( x < 0.0000000001 )
				SetColor(White, Black);
			else
				SetColor(Blue, Green);
			cout<<setprecision(1)<<x;
			SetColor(White, Black);
		}
		cout<<"\n";
		SetColor(White, Black);
	}
	cout<<"\n";
	for ( int i = 0; i < rn; ++i)
	{
		for ( int j = 0; j < cn; ++j)
		{
			cout<<setw(3);
			double x = f[i * cn + j + rn*cn];
			if ( x < 0.0000000001 )
				SetColor(White, Black);
			else
				SetColor(Blue, Green);
			cout<<setprecision(1)<<x;
			SetColor(White, Black);
		}
		cout<<"\n";
		SetColor(White, Black);
	}*/
	cout<<"___________________\n";
	system("pause");
}
#define show(x, y)

/*void show(const double * const f)
{
	cout<<"___________________\n";
	for ( int i = 0; i < 28*2+1; ++i)
	{
		for ( int j = 0; j < 50 - (i%2==0); ++j)
		{
			if ( i % 2 == 0 )
			{
				SetColor(Red, Black);
				cout<<setw(2)<<"P";
				SetColor(White, Black);
			}
			double x = f[28*50*2 + ( (i%2==0) ? ((i/2)*(50-1) + j) : ((i/2)*50 + j + (50-1)*28) ) ];
			if ( x < 0.000000000001 )
				SetColor(White, Black);
			else
				SetColor(Blue, Green);
			cout<<setw((i%2==0)?3:5)<<setprecision(2)<<x;
			SetColor(White, Black);
		}
		if ( i % 2 == 0 )
		{
			SetColor(Red, Black);
			cout<<setw(2)<<"P";
			SetColor(White, Black);
		}
		cout<<"\n\n";
		SetColor(White, Black);
	}
	cout<<"-------------------\n";
	for ( int i = 0; i < 28; ++i)
	{
		for ( int j = 0; j < 50; ++j)
		{
			cout<<setw(3);
			double x = f[i * 50 + j];
			if ( x < 0.000000000001 )
				SetColor(White, Black);
			else
				SetColor(Blue, Green);
			cout<<x;
			SetColor(White, Black);
		}
		cout<<"\n";
		SetColor(White, Black);
	}
	cout<<"\n";
	for ( int i = 0; i < 28; ++i)
	{
		for ( int j = 0; j < 50; ++j)
		{
			cout<<setw(3);
			double x = f[i * 50 + j + 28*50];
			if ( x < 0.000000000001 )
				SetColor(White, Black);
			else
				SetColor(Blue, Green);
			cout<<x;
			SetColor(White, Black);
		}
		cout<<"\n";
		SetColor(White, Black);
	}
	cout<<"___________________\n";
	system("pause");
}*/

#else

#define DEBUG(x)
#define dout(x)
#define show(x, y)
#define showm(x)

/*
template<typename ...Arg>
__forceinline void dout(Arg... arg)
{}
*/

#endif // _DEBUG_BUILD_
#endif // __DEBUG_HPP__
