#include <iostream>
#include <cmath>
#include <fstream>
#include <limits.h>
#include <vector>
#include <map>
using namespace std;

const int MAX_P = 4;	//4 because we have to manually check the correct answer, and that takes a while.
int dp[1024];	// Saves f(S) for a given mask. (The masks <1024 => # of people less than 10. )
int n;				// The number of people.
int t[MAX_P];	// The time taken for an individual person to cross the bridge.
int C;				// The capacity of a boat.
/*
* Evaluates the complement of a given set S (mask s).
* Complement is the value 1111..MAX_P times - s.
*/
int comp(int s){
	int p = pow(2,MAX_P) -1;
	return p - s;
}
/*
* Evaluates union of two sets s, g.
* This is the bitwise OR of the two masks.
*/
int uni(int s, int g){
	return s|g;
}
/*
* Removes the elements of G from S.
* This works because G is always a subset of S (in our case).
*/
int negation(int s,int g){
	return s - g;
}

/*
*	makes a binary representation for a singleton set
*	containing a person i.( indexed from 0), which is simply 2^i.
*/
int make_set(int i){
	return pow(2,i);
}

// This is the number of 1's in the binary representation of the set.
int cardinality(int s){
	return __builtin_popcount(s);
}

/*
*	Selects from and converts a specific set of an array of integers to a mask representing the array.
* The values array is used to know what all elements are to be included in the desired mask.
*/
int arrtomask(vector<int> m,bool values[]){
	int mask = 0;
	int size = m.size();
	for(int i=size-1;i>=0;i--){
		if(values[i])
			mask+=pow(2,m[i]-1);
	}
	return mask;
}

vector<int> sets;	// A global variable which shall be used to store all subsets of a fixed size of a given set. This is used later in subsets().

/*
* Finds all subsets of a given array. The result is stored in the vector 'ret'.
* k => required size of the subset.
* mask => the mask of the original array from which subsets are to be calculated.
* This proceeds in the following fashion:
* At each stage, we can choose whether to include an element in our set or discard it.
* If included, then we increase the current size by 1, otherwise we don't.
*
*/
void subsets(vector<int> mask,int k,int start, int currlen, bool used[]){
	if(currlen == k){
		sets.push_back(arrtomask(mask,used));
		return ;
	}
	if(start == mask.size()){
		return ;
	}
	used[start] = true;									// include the element
	subsets(mask,k,start + 1, currlen + 1, used);

	used[start] = false;								//do not include the element
	subsets(mask,k,start+1, currlen, used);
}

/*
* Converts a mask to an array containing the indices of 1's in the mask.
* eg. 1101 is converted to {1,3,4}. (Traversing in reverse order.)
*/
vector<int> masktoarr(int mask){
	int pos = 0;
	vector<int> v;
	while(mask>0){
		pos++;
		int i = mask%2;										// Check if ith bit is 1
		if(i){
			v.push_back(pos);
		}
		mask/=2;
	}
	return v;
}


/*
* The fastest person in a given set S.
* In the binary representation of S,
* 	if bit at index i == 1,
* 		check if the person has speed less than the current min speed.
*/
int fastest(int s){
	int i = 0;
	int f = 1,min_t = INT_MAX;
	while(s>0){
		int k = s%2;
		if(k){
			if(t[i]<min_t){
				min_t = t[i];
				f = i;
			}
		}
		i++;
		s /=2;
	}
	return f+1;
}

/*
*	returns the time taken to transfer the set
*	completely from the South to North bank.
* This is equal to the time taken to cross the bridge,
* by the slowest person in the set.
*/
int complete_time(int s){
	int i = 0;
	int f = 0,max_t = INT_MIN;
	while(s>0){
		int k = s%2;
		if(k){
			if(t[i]>max_t){
				max_t = t[i];
				f = i;
			}
		}
		i++;
		s /=2;
	}
	return t[f];
}

/*
*	next(S,G) is given by (S-G) U { fastest( (P-S) U G) }
*/
int next(int s, int g){
	int people_north = uni(comp(s),g);
	int returning_person = fastest(people_north) - 1;				// this is because the time array is indexed with 0 instead of 1.
	return uni(negation(s,g), make_set(returning_person));			// But fastest(s) returns answers starting from index 1.
}

/*
*	The time taken to move the subset G of S from South to north and
*	return the fastest person on the northern bank with the torch.
* 	which is complete_time(G) + fastest in (P-S) U G .
*/
int T(int s,int g){
	int people_north = uni(comp(s),g);
	return complete_time(g) + t[fastest(people_north)-1];
}

/*
* The time taken to transfer a set S from the south bank to the north bank.
* The function is defined later.
*/
int f(int s);

/*
*	Returns all subsets of a set S which satisfy the feasibility condition, that |G|=C.
*/
void feasible(int s){
	sets.clear();
	vector<int> indices = masktoarr(s);
	bool used[indices.size()] ;									// The array is not initialized here, since variable size array initialization
	 																						// causes errors on compilers running on version 4.8 or older.
	for(int i=0;i<indices.size();i++)
		used[i]=0;

	subsets(indices,C,0,0,used);
}

map<int,vector<int> > opt_next;               // Stores the subset that travels across the bridge at each stage. This is needed to print the entire solution.

/*
* The computaion of time for each subset is done here.
*/
int genf(){
	int p = 1;
	for(int i=0;i<MAX_P;i++){
		dp[p] = t[i];				               			// All singleton sets. Their binary representations have only 1 bit as 1, all others are zeros.
		p*=2;
	}
	for(int i = 1;i<p;i++){
		if(dp[i]==0){
			if(cardinality(i)<=C){
				dp[i] = complete_time(i);           // All sets with cardinality less than or equal to C can be transferred directly.
			}
			else{
				feasible(i);
				int minT = INT_MAX;
				for(int j=0;j<sets.size();j++){
					int currG = sets[j];
					int currT = T(i,currG) + dp[next(i,currG)];    // for the remaining sets we need to find the set which takes the minimum time to travel across.
					if(currT<minT){
						minT = currT;
					}
				}

				vector<int> opt_solu;
				for(int j = 0;j<sets.size();j++)
				{
					if(T(i,sets[j]) + dp[next(i,sets[j])] == minT){
						opt_solu.push_back(sets[j]);
					}
				}
				opt_next[i] = opt_solu;
				dp[i] = minT;
			}
		}
	}
}


int f(int s){
	return dp[s];
}
/*
* Input format:
* The input is read from input.txt, and is given as follows.
* The first line of the input contains the travel time of each of the four people standing on the southern bank.
* Second line contains two integers C, Q.
* C => Capacity of the boat.
* Q => The set representation of the people on the southern bank. (We always assume that MAX_P people are present in the game. So a set {2,4}
*       would imply that {1,3} are on the northern bank. )
* Q is given in the form of a mask, which is calculated as:
* Convert the set to a binary representation. Convert this binary representation to a decimal.
* eg. {1,2,4} is represented as 1011 (right to left) which is equal to the decimal 11 = Q.
*
*/
int main(){
  ifstream fin;
  ofstream fout;
  fin.open("input.txt",ios::in);
  fout.open("output.txt",ios::out);
  for(int i=0;i<4;i++){
		fin>>t[i];
	}
	int query;

	fin>>C>>query;
  fin.close();
	genf();
	fout<<"The optimal time taken is "<<dp[query]<<" units."<<endl;
	fout<<"The solution is as follows:\n";

	int i = query;
	while(true){                                                       // This prints the optimal solution.
		if(opt_next[i].size()<1)
		{
			fout<<"-->"<<i<<endl;
			break;
		}
		fout<<"-->"<<opt_next[i][0];
		i = next(i,opt_next[i][0]);
	}
	fout<<"Each of the numbers above represents the mask of the set that was sent.\n";
	fout<<"eg. 3 = \"0011\" => {1,2} is the required set. (Looking from the right \n in the binary representation.) \n";
  fout.close();
  return 0;
}
