#include <iostream>


/*
* Assume 32-bit x86 windows machine
* sizeof(unsigned short):2
* sizeof(int):4
* sizeof(long):4
* sizeof(char*):4
*/
struct MyStruct
{
   unsigned short v1;    // 0 bytes offset
   unsigned short v2;    // 2 bytes offset
   unsigned short v3;    // 4 bytes offset
} ;// End 6 bytes.


// No part is required to align tighter than 2 bytes. 
// So whole structure can be 2 byte aligned.

struct MyStruct2
{
    unsigned short v1;      // 0 bytes offset
    unsigned short v2;      // 2 bytes offset
    unsigned short v3;      // 4 bytes offset
    /// Padding             // 6-7 padding (so i is 4 byte aligned
    int i;                  // 8 bytes offset
}; // End 12 bytes       

struct MixedData
{
    char Data1; 		/* 1 byte */
    char Padding1[1]; 	/* 1 byte for the following 'short' to be aligned on a 2 byte boundary
						assuming that the address where structure begins is an even number */
    short Data2; 		/* 2 bytes */
    int Data3;  		/* 4 bytes - largest structure member */
    char Data4; 		/* 1 byte */
    char Padding2[3]; /* 3 bytes to make total size of the structure 12 bytes */
}; // End of 12 bytes

struct Foo1 {
    char *p;	// 4 bytes offset
    char c;     // 1 bytes, 3 bytes padding
    long x;     // 8 bytes offset
}; // End of 12 bytes

struct Foo2 {
    char *p;     /* 8 bytes */
    char c;      /* 1 byte */
};

struct Foo3 {
    char c;
    struct foo5_inner {
        char *p;
        short x;
    } inner;
};

struct StructExample {
	char c;  // 0 bytes offset, 3 bytes padding
	int i;   // 4 bytes offset
	short s; // 8 bytes offset, 2 bytes padding
}; // End of 12 bytes

int main()
{

    std::cout << "sizeof(unsigned short):" << sizeof(unsigned short) << std::endl;
    std::cout << "sizeof(short):" << sizeof(short) << std::endl;
    std::cout << "sizeof(int):" << sizeof(int) << std::endl;
    std::cout << "sizeof(long):" << sizeof(long) << std::endl;
    std::cout << "sizeof(char*):" << sizeof(char*) << std::endl;


	MyStruct myStruct;
	MyStruct2 myStruct2;
	MixedData mixData;
	Foo1 foo1;
	Foo2 foo2;
	Foo3 foo3;
	StructExample sExample;

	std::cout << "sizeof(MyStruct): " << sizeof(myStruct) << std::endl;
	std::cout << "sizeof(MyStruct2):" << sizeof(myStruct2) << std::endl;
	std::cout << "sizeof(MixedData):" << sizeof(mixData) << std::endl;
	std::cout << "sizeof(Foo1):" << sizeof(foo1) << std::endl;
	std::cout << "sizeof(Foo2):" << sizeof(foo2) << std::endl;
	std::cout << "sizeof(Foo3):" << sizeof(foo3) << std::endl;
	std::cout << "sizeof(StructExample):" << sizeof(sExample) << std::endl;
	std::system("pause");
	return 0;
}


/*
on x86_64 linux
sizeof(unsigned short):2
sizeof(short):2
sizeof(int):4
sizeof(long):8
sizeof(char*):8
sizeof(MyStruct): 6
sizeof(MyStruct2):12
sizeof(MixedData):12
sizeof(Foo1):24
sizeof(Foo2):16
sizeof(Foo3):24
*/

/*
on x86 64 bit windows
sizeof(unsigned short):2
sizeof(short):2
sizeof(int):4
sizeof(long):4
sizeof(char*):4
sizeof(MyStruct): 6
sizeof(MyStruct2):12
sizeof(MixedData):12
sizeof(Foo1):12
sizeof(Foo2):8
sizeof(Foo3):12
*/

/*
struct Foo3 {
    char c;           // 1 byte
    char pad1[7];     // 7 bytes 
    struct foo5_inner {
        char *p;      // 8 bytes 
        short x;      // 2 bytes 
        char pad2[6]; // 6 bytes 
    } inner;
};
*/