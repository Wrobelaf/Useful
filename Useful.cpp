#include "StdAfx.h"
#include <math.h>

using namespace std;


int  NUM_INVALID =		0x01;
int  NUM_DEC	 =		0x02;
int  NUM_HEX	 =		0x04;
int  NUM_REAL	 =		0x08;


static int	optmyerr = 1;
static int	optmyind = 1;
static int	optmyopt = 0;
char	*optmyarg    = 0;

string trim(const string& s,const string& drop = " \t")
{
	string r=s;
	r=r.erase(r.find_last_not_of(drop)+1);
	return r.erase(0,r.find_first_not_of(drop));
}

vector<string>& Tokenize(const string& str,
						 vector<string>& tokens,
                         const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
	return tokens;
}

unsigned int GetNum (const char * const itm,double &d)
{
	// convert string 'itm' to a number and return via 'd'.
	// input string can be a decimal (+/-)integer, hex integer (leading 0x) or a real number +/-nn.mmm
	// return a bitsword giving the status and type of result.
	//	NUM_INVALID set if invalid format
	//  NUM_DEC		set if the number was a decimal number.
	//  NUM_HEX		set if the number was hexadecimal.
	//  NUM_REAL	set if the number was a real number.

	int n      = 0;
	double frac= 0.0;
	int pow    = 10;
	int sign   = 1;
	int state  = 0;
	int idx    = 0;
	unsigned int bw = 0;

	static const char hex[] = "0123456789ABCDEF";
											//0 n  X  .    eothex - rest
	static const unsigned char fsm[][9] =   {{1,6, 4,10, 0,10,10, 3,10},  // 0
											 {6,6, 4, 7,10, 9,10,10,10},  // 1
											 {6,6,10,10, 2,10,10,10,10},  // 2
											 {6,6,10,10, 3,10,10,10,10},  // 3
											 {5,5,10,10,10,10, 5,10,10},  // 4
											 {5,5,10,10,10, 9, 5,10,10},  // 5
											 {6,6,10, 7,10, 9,10,10,10},  // 6
											 {8,8,10,10,10,10,10,10,10},  // 7
											 {8,8,10,10,10, 9,10,10,10}}; // 8
	do
	{
		char c= itm[idx++];
		state = fsm[state][(c == '0')   ? 0 :
						   (isdigit(c)) ? 1 :
						   ((c == 'x') || (c == 'X')) ? 2 :
						   (c == '.')   ? 3 :
						   (isspace(c)) ? 4 :
						   (c == '\0')  ? 5 : 
						   (isxdigit(c))? 6 :
						   (c == '-')   ? 7 : 8];
		switch (state)
		{
		case 0:
		case 1:
		case 2:
			break;
		case 4:
			bw |= NUM_HEX;
			break;
		case 3:
			sign = -1;
			break;
		case 5:
		{
			char *pchr = strchr(hex,toupper(c));
			n = (n*16)+(pchr-hex);
			break;
		}
		case 6:
			bw |= NUM_DEC;
			n = (n*10)+(c-'0');
			break;
		case 7:
			bw |= NUM_REAL;
			break;
		case 8:
			frac += ((double)(c-'0'))/pow;
			pow  *= 10;
			break;
		case 9:									// Success got a number
			d = (n+frac)*sign;
			break;
		case 10:
			bw = NUM_INVALID;
			break;
		}
	} while (state < 9);

	return bw;
}

// No standard 'getopt' across operating systems, so rip one off the net.

int getmyopt(int argc, char **argv, const char * const opts)
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optmyind >= argc ||
		   argv[optmyind][0] != '-' || argv[optmyind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optmyind], "--") == 0) {
			optmyind++;
			return(EOF);
		}
	optmyopt = c = argv[optmyind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == 0)
	{
		if(argv[optmyind][++sp] == '\0') {
			optmyind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optmyind][sp+1] != '\0')
			optmyarg = &argv[optmyind++][sp+1];
		else if(++optmyind >= argc) {
			sp = 1;
			return(':');
		} else
			optmyarg = argv[optmyind++];
		sp = 1;
	} else {
		if(argv[optmyind][++sp] == '\0') {
			sp = 1;
			optmyind++;
		}
		optmyarg = 0;
	}
	return(c);
}

static unsigned int i_nines_complement (unsigned int n,int &ndigits)
{
	// return the 'nines complement' of 'n', on entry n != 0 and the number of digits of the number

	unsigned int comp= 0;
	int          pow = 1;
	
	ndigits = 1;
		
	while (n != 0)
	{
		unsigned int t = (9-(n % 10))*pow;
		n   /= 10;
		pow *= 10;
		comp+= t;
		if (pow > 10)
			ndigits++;
	}

	return comp;
}

int ndigits (unsigned int n)
{
	//return the number of digits of 'n'

	int ndig;
    i_nines_complement (n,ndig);

	return ndig;
}

double round_to (double d,int amo)
{
	// Round 'd' to 'amo' number of digits

	return floor(d*(double)amo+0.5)/(double)amo;
}

unsigned int nines_complement (unsigned int n)
{
	// return the 'nines complement' of 'n', on entry n != 0 and the number of digits of the number

	int ndig;
    return i_nines_complement (n,ndig);
}

int normalise (double n,unsigned int & i,bool & sign,bool &nan)
{
	// Geven real 'n' converts it to abs(integer) returning the power to divide (-ve) or multiply it (+ve)
	// also returns the sign of the original 'n'
	// e.g.  12.0   converted to 12 with power=0
	//       12.7   converted to 127 with power=-1
	//        0.00127 converted to 127 with power=-5
	//    12700.0     converted to 127 with power=2

	int  retpow     = 0;
	int  integerpow = 0;
	int  fracpow    = -1;
	int  savedfrac  = 0;
	int  state      = 0;
	int  idx        = 0;

	nan = sign = false;
	if (n < 0)
	{
		sign = true;
		n = fabs(n);
	}

	static const unsigned char fsm [][5] = {{0,1,3,7,7},		// 0
										    {2,1,3,7,7},		// 1
											{2,1,3,7,7},		// 2
											{5,4,7,7,7},		// 3
											{5,4,7,6,7},		// 4
											{5,4,7,6,7}};		// 5
	char sf[21];

	sprintf (sf,"%-20.10f",n);

	while (state < 6)
	{
		char c = sf[idx++];

		state = fsm[state][(c == '0') ? 0 :
						   ((c >= '1') && (c <= '9')) ? 1 :
						   (c == '.') ? 2 :
						   ((c == ' ') || (c == '\0')) ? 3 : 4];
		switch (state)
		{
		case 0:
		case 3:
			break;
		case 1:
			integerpow = 0;
			break;
		case 2:
			integerpow++;
			break;
		case 4:
			savedfrac = fracpow;			// fall through
		case 5:
			fracpow--;
			break;
		case 6:
			retpow = integerpow = (savedfrac != 0) ? savedfrac : integerpow;
			if (integerpow < 0)
			{
				while (integerpow++ != 0)
					n *= 10;
			}
			else if (integerpow > 0)
			{
				while (integerpow-- != 0)
					n /= 10;
			}
			break;
		case 7:						// NAN
			nan = true;
			break;
		}
	}
	if (!nan)
	{
		if (n > 4294967295.0)			// 2^32-1
		{
			nan = true;
		}
		else
		{
			i = (unsigned int) (n + 0.5);
		}
	}
	return retpow;
}


