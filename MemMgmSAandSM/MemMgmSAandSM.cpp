// MemMgmSAandSM.cpp : Completed by Weiwei Shi

#include<iostream>
#include<cstdlib>
#include<cassert>
#include<string>
#include<stdio.h>
#include<stddef.h>
using namespace std;

//Tags for SA memory allocations
struct Head {
	unsigned len = 0;
	
	//Head pointer points to the next free mem. in linked list
	struct Head* freepointer = 0; 
};

//object of Head struct to hold the free mem. link-list
static Head freeList;


//typedef used to convert raw address to bytes for easier memory calculations
typedef char* Char_p;

//Template safe array class
template <class T>
class SA {
private:
	int clow, chigh;
	T* p;

public:
	//default constructor
	//allows for writing things like SA<int> a;
	SA<T>() {
		clow = 0;
		chigh = -1;
		p = nullptr;
	}

	//single parameter constructor lets us create a SA
	//almost like a "standard" one by writing SA<string> x(10);
	//and getting an array x indexed from 0 to 9
	SA<T>(int c) {
		clow = 0;
		chigh = c - 1;
		p = new T[c];
	}

	//2parameter constructor lets us write SA<double> x(10,20);
	SA<T>(int cl, int ch) {
		if ((ch - cl + 1) <= 0) {
			cout << "constructor error in bounds definition" << endl;
			exit(1);
		}
		clow = cl;
		chigh = ch;
		int csize = ch - cl + 1;
		p = new T[csize];
	}

	//copy constructor for pass by value and initialization
	SA<T>(const SA<T>& s) {
		int csize = s.chigh - s.clow + 1;

		p = new T[csize];
		for (int j = 0; j < csize; j++)
			p[j] = s.p[j];

		clow = s.clow;
		chigh = s.chigh;
	}

	//copy move constructor for when value passed is going out of
	//scope right after
	SA<T>(SA<T>&& s) {
		p = s.p;
		s.p = nullptr;

		clow = s.clow;
		chigh = s.chigh;
	}

	//destructor
	~SA<T>() {
		delete[] p;
	}

	//overloaded[] lets us write SA<int> x(10, 20); x[15] = 100;
	T& operator[](int c) {
		if (c<clow || c>chigh) {
			cout << "index " << c << " out of range" << endl;
			exit(1);
		}
		return p[c - clow];
	}

	//overloaded opeator= lets us write SA<int> x(10, 20), y(1, 2); x = y;
	SA<T>& operator= (SA<T>& s) {
		int size = s.chigh - s.clow + 1;
		if (this == &s) {
			return *this;
		}

		p = new T[size];
		for (unsigned i = 0; i < size; i++) {
			p[i] = s.p[i];
		}

		clow = s.clow;
		chigh = s.chigh;
		return *this;
	}

	//move assignment operator for when object assigned from will be 
	//out of scope right after
	SA<T>&& operator= (SA<T>&& s) {
		if (this == &s) {
			return move(*this);
		}

		p = s.p;
		s.p = nullptr;

		clow = s.clow;
		chigh = s.chigh;
		return move(*this);
	}

	//operator new to manually allocate and reuse memory when new is called
	//to initialize an object of class SA
	void* operator new(size_t size) {

		//if free mem. exist in the freeList, look for one that fits the size
		//and return the address after the Head information
		if (freeList.freepointer) {
			Head* prev = &freeList;
			for (Head* cur = &freeList; cur; cur = cur->freepointer) {
				if (cur->len >= size) {
					//if the next free space is the rest of the "heap", carve out needed space
					//and link the rest back to the freeList
					if (static_cast<int>(cur->len - size - sizeof(Head)) >= 12) {
						Head* rest = (Head*)(Char_p(cur) + sizeof(Head) + size);
						rest->len = cur->len - (size + sizeof(Head));
						rest->freepointer = 0;
						cur->freepointer = rest;
						cur->len = size;
						
					}
					prev->freepointer = cur->freepointer;
					return (void*)(Char_p(cur) + sizeof(Head));
				}
				else
					prev = cur;
			}
		}
		//if no suitable space is found in freeList, ask for a big chunk of new space, set its
		//Head information
		Head *newSpace = (Head*) ::operator new(sizeof(Head) * 10000);
		newSpace->len = size;
		newSpace->freepointer = 0;

		//carve out the required amount of space at the beginning, link the rest of the free space 
		//with freeList
		Head* rest = (Head*)(Char_p(newSpace) + sizeof(Head) + size);
		freeList.freepointer = rest;
		rest->len = sizeof(Head) * 9998 - size;
		rest->freepointer = 0;

		//return the address of the carved out space after the header info
		return (void*)(Char_p(newSpace) + sizeof(Head));
	}

	//a different operator new for the same purpose as above but gets
	//called when an array of SA is being initialized
	void* operator new[](size_t size) {
		//the code in here is identical to the one above
		if (freeList.freepointer) {
			Head* prev = &freeList;
			for (Head* cur = &freeList; cur; cur = cur->freepointer) {
				if (cur->len >= size) {
					if (static_cast<int>(cur->len - size - sizeof(Head)) >= 12) {
						Head* rest = (Head*)(Char_p(cur) + sizeof(Head) + size);
						rest->len = cur->len - (size + sizeof(Head));
						rest->freepointer = 0;
						cur->freepointer = rest;
						cur->len = size;
					}

					prev->freepointer = cur->freepointer;
					return (void*)(Char_p(cur) + sizeof(Head));
				}
				else
					prev = cur;
			}
		}
		//if no suitable space is found in freeList, ask for a big chunk of new space, set its
		//Head information
		Head* newSpace = (Head*) ::operator new(sizeof(Head) * 10000);
		newSpace->len = size;
		newSpace->freepointer = 0;

		//carve out the required amount of space at the beginning, link the rest of the free space 
		//with freeList
		Head* rest = (Head*)(Char_p(newSpace) + sizeof(Head) + size);
		freeList.freepointer = rest;
		rest->len = sizeof(Head) * 9998 - size;
		rest->freepointer = 0;

		//return the address of the carved out space after the header info
		return (void*)(Char_p(newSpace) + sizeof(Head));
	}

	//operator delete to manually return memory to freeList for possible reuse later
	void operator delete(void* ptr) {

		//use ptr address, go back the size of Head to get its Head information
		Head* hp = (Head*)(Char_p(ptr) - sizeof(Head));

		//adds mem. to the front of freeList
		hp->freepointer = freeList.freepointer;
		freeList.freepointer = hp;
	}

	//another operator delete for the same purpose as above but gets called
	//when an array of SA is beeing deleted
	void operator delete[](void* ptr) {

		//the code here is identical to the one above
		Head* hp = (Head*)(Char_p(ptr) - sizeof(Head));
		hp->freepointer = freeList.freepointer;
		freeList.freepointer = hp;
	}

	//overloads << so we can directly print SAs
	template<typename U>
	friend ostream& operator<<(ostream& os, SA<U>& s);
};
template <typename T>
ostream& operator<<(ostream& os, SA<T>& s) {
	int csize = s.chigh - s.clow + 1;
	for (int i = 0; i < csize; i++) {
		cout << s.p[i] << endl;
	}
	return os;
}


//template Safe Matrix class
template<class S>
class SM {
private:
	int rlow, rhigh;
	int clow, chigh;
	//stores the pointer to safe array of safe arrays
	SA<SA<S>>* SAofSA;
public:
	//default constructor allows us to write things like SM<int> matrix;
	SM() {
		rlow = 0;
		rhigh = -1;
		clow = 0;
		chigh = -1;
		*SAofSA = nullptr;
	}

	//2 parameter constructor allows us to write like SM<int> matrix(2, 3);
	SM(int r, int c) {

		SAofSA = new SA< SA<S> >(r);
		for (int i = 0; i < r; i++) {
			SA<S>* s = new SA<S>(c);
			(*SAofSA)[i] = move(*s);
			delete s;
		}

		rlow = 0;
		rhigh = r - 1;
		clow = 0;
		chigh = c - 1;
	}

	//4 parameter constructor allows us to write things like
	//SM<int> s (2,3,3,5); and s[3][5];
	SM(int rl, int rh, int cl, int ch) {
		int rowSize = rh - rl + 1;
		if (rowSize <= 0) {
			cout << "constructor error in row bounds definition" << endl;
			exit(1);
		}

		SAofSA = new SA<SA<S>>(rl, rh);
		for (int i = rl; i <= rh; i++) {
			SA<S> *s = new SA<S>(cl, ch);
			(*SAofSA)[i] = move(*s);
			delete s;
		}
	
		rlow = rl;
		rhigh = rh;
		clow = cl;
		chigh = ch;
	}
	
	//copy constructor for pass by value and initialization
	SM<S>(const SM<S>& s) {

		int rsize = s.rhigh - s.rlow + 1;
		int csize = s.chigh - s.clow + 1;

		SAofSA = new SA< SA<S> >(rsize);
		for (int i = 0; i < rsize; i++) {
			(*SAofSA)[i] = (*(s.SAofSA))[i];
		}
		rlow = s.rlow;
		rhigh = s.rhigh;
		clow = s.clow;
		chigh = s.chigh;
	}

	//destructor
	~SM() {
		delete SAofSA;
	}

	//operator [] overload allows us to write things like matrix[1][0];
	SA<S>& operator[](int r) {
	return (*SAofSA)[r];
	}

	//overloads assignment operator so we can write things like matrix1 = matrix2;
	SM<S>& operator=( SM<S>& s) {
		if (this == &s)return *this;
		delete SAofSA;
		SAofSA = new SA<SA<S>>(s.rlow, s.rhigh);
		for (int i = s.rlow; i <= s.rhigh; i++) {
			(*SAofSA)[i] = (*(s.SAofSA))[i];
		}
		rhigh = s.rhigh;
		rlow = s.rlow;
		chigh = s.chigh;
		clow = s.clow;
		return *this;
	}

	SM<S>* operator*(SM<S> const& other)const;

	//overloads << so we can directly print SMs
	template<typename V>
	friend ostream& operator<<(ostream& os, SM<V>& s);
};
template <typename S>
ostream& operator<<(ostream& os, SM<S>& s) {
	for (int i = s.rlow; i < s.rhigh + 1; i++) {
		for (int j = s.clow; j < s.chigh + 1; j++)
			cout << (*(s.SAofSA))[i][j] << " ";
		cout << endl;
	}
	return os;
}

//overloads operator * so we can do matrix multiplication like matrix1 * matrix2
template <typename S>
SM<S>* SM<S>::operator*(SM<S> const& other) const {
	int cSize = chigh - clow + 1;
	int rSize = rhigh - rlow + 1;
	int otherCS = other.chigh - other.clow + 1;
	int otherRS = other.rhigh - other.rlow + 1;

	//initialize the result matrix with dimensions according to the original 
	//matrix operands
	int ansRow = 0, ansCol = 0;
	if (cSize == otherRS) {
		SM<S> ansMatrix(cSize, otherRS);
		ansRow = rSize;
		ansCol = otherCS;
	}
	else if (rSize == otherCS) {
		SM<S> ansMatrix(rSize, otherCS);
		ansRow = rSize;
		ansCol = otherCS;
	}
	else {
		cout << "# of columns and # of rows of matrix don't match." << endl;
		exit(1);
	}

	//multiply two matrix element by element and stor in result matrix
	SM<S>* ansMatrix = new SM<S>(ansRow, ansCol);
	for (int r = 0; r < rSize; r++) {
		for (int c = 0; c < otherCS; c++) {
			(*ansMatrix)[r][c] = 0;

			for (int k = 0; k < otherRS; k++) {
				(*ansMatrix)[r][c] += (*SAofSA)[r + rlow][k + clow] * (*other.SAofSA)[k + other.rlow][c + other.clow];
			}
		}
	}
	return ansMatrix;
}

int main()
{
	SM<string> s(2, 2);
	s[0][0] = "this is the [0][0] string";
	s[0][1] = "this is the [0][1] string";
	s[1][0] = "this is the [1][0] string";
	s[1][1] = "this is the [1][1] string";
	cout << "Printing String Matrix: " << endl << s << endl;

	SM<string>copy(s);
	cout << "this is a copy of s: " << endl << copy << endl;

	SM<int> matrix(2, 3, 3, 5);
	matrix[2][3] = 1;
	matrix[2][4] = 2;
	matrix[2][5] = 3;
	matrix[3][3] = 4;
	matrix[3][4] = 5;
	matrix[3][5] = 6;
	cout << "matrix using 4 param. constructor:" << endl << matrix << endl;

	SM<int>* matrix2 = new SM<int>(2, 3, 3, 5);
	(*matrix2)[2][3] = 3;
	(*matrix2)[2][4] = 4;
	(*matrix2)[2][5] = 2;
	(*matrix2)[3][3] = 3;
	(*matrix2)[3][4] = 4;
	(*matrix2)[3][5] = 2;

	cout << "matrix2: " <<endl << *matrix2 << endl;

	SM<int>* matrix3 = new SM<int>(3, 4);
	(*matrix3)[0][0] = 13;
	(*matrix3)[0][1] = 9;
	(*matrix3)[0][2] = 7;
	(*matrix3)[0][3] = 15;
	(*matrix3)[1][0] = 8;
	(*matrix3)[1][1] = 7;
	(*matrix3)[1][2] = 4;
	(*matrix3)[1][3] = 6;
	(*matrix3)[2][0] = 6;
	(*matrix3)[2][1] = 4;
	(*matrix3)[2][2] = 0;
	(*matrix3)[2][3] = 3;

	cout << "matrix3: " << endl << *matrix3 << endl;

	SM<int> *answer = (*matrix2)*(*matrix3);
	cout << "matrix2 * matrix3:" << endl << *answer << endl;

	SM<int>* matrix4 = new SM<int>(5, 6);

	*matrix4 = *matrix2;
	cout << "now matrix4 is a copy of matrix2: " << endl << *matrix4 << endl;

	cout << "deleting matrix2, matrix3, matrix4, and answer" << endl;
	delete matrix2;
	delete matrix3;
	delete matrix4;
	delete answer;

	char charType = 'G';
	printf("size of char datatype is: %ld byte \n", sizeof(charType));

	SA<int>* test = new SA<int>(3);
	(*test)[0] = 4;
	(*test)[1] = 4;
	(*test)[2] = 4;
	cout << "Safe Array type int test:" << endl << *test << endl;

	cout << "deleting SA test" << endl;
	delete test;

	SA<int>* test2 = new SA<int>(3);
	(*test2)[0] = 4;
	(*test2)[1] = 4;
	(*test2)[2] = 4;
	cout << "Safe Array type int test:" << endl << *test2 << endl;

	cout << "deleting SA test2" << endl;
	delete test2;

	SA<char> doesThisWork(2);
	doesThisWork[1] = 'c';
	doesThisWork[0] = 'd';
	cout << "Safe Array type char test:" << endl << doesThisWork << endl;

	SA<int> a(10), b(3, 5);
	b[3] = 3; b[4] = 4; b[5] = 5;
	int i;
	for (i = 0; i < 10; i++)
		a[i] = 10 - i;
	cout << "printing a the first time" << endl;
	cout << a << endl;

	cout << "printing using[]" << endl;
	for (i = 0; i < 10; i++)
		cout << a[i] << endl;

	cout << "printing a the second time" << endl;
	cout << a << endl;

	b[4] = 12;
	cout << "printing b " << endl;
	cout << b << endl;
	//a[10] = 12; // should print an error message and exit
	return 0;
}

