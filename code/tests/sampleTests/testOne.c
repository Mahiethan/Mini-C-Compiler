// works
// #include <stdbool.h>
//cant have func decl before extern
// int lineOne(int a, float b)
// {
//     return a;
// }




// extern void lineTwo(int a, float b);
// extern void lineTwo(int a, float b;





// extern int lineThree(int a, bool b); 
// int testOne; //must have some decl after extern
// int testTwo; //must have some decl after extern
// // void testBool(int f, float c)
// // {
// //     //variable declarations must go BEFORE any expressions
// //     int a; int b;
// //     a = 1;
// //     // a = b = 5 - (-!7) / 7 + 2 * 3;
// //     a = (5 + 3 / 3) * 3 % 3;
// //     // b = lineTwo(2+3/3-2,3%2);
// //     // float b;
// //     ;
// //     ;
// //     ;
// //     {

// //     }
// //     if(a == 1)
// //     {
// //         b = 2;
// //     }
// //     else
// //     {
// //         b = 3;
// //     }
// //     //b = 1;
// //     //int a = 1; //invalid
// //     return a;
// // }
// void qr(int a, int b)
// {
//     int a;
//     //bottom alone not allowed!!
//     //int a = 1;
//     //a = 1;
//     // while(a == 3)
//     // {
//     //     int b;
//     //     a = 123;
//     //     if(a == 123)
//     //     {
//     //         a = 3;
//     //     }
//     //     if(a == 3)
//     //     {
//     //         a = 1;
//     //         while(a == 1)
//     //         {
//     //             return a = 5;
//     //         }
            
//     //     }
//     //     else
//     //     {
//     //         t = 19;
//     //         if(t == 19)
//     //         {
//     //             a == 10;
//     //         }
//     //         else
//     //         {
//     //           a = 10;  
//     //         }
//     //         while(t == 3)
//     //         {
//     //             if(r == 2)
//     //             {

//     //             }
//     //             return 10;
//     //         }
          
//     //     }
//     //     return false;
//     // }
//     // while(a == 1)
//     // {
//     //     int b;
//     //     a = 3;
//     // }
//     // if(a == 3)
//     // {
//     //     // if(a == 1)
//     //     // {
//     //     //     int a;
//     //     //     return 100;
//     //     // }
//     //     // else
//     //     // {
//     //     //     if(a == 100)
//     //     //     {
//     //     //      test(a = 4+3,!3);
//     //     //      return 10;
//     //     //     }
//     //     // }
//     // }
//     // while(a == 0)
//     // {
//     //     // if(a == 0)
//     //     // {

//     //     // }
//     //     // else
//     //     // {
//     //     // //    a = 1;
//     //     // }        
//     // }
//     // if(func(3,4))
//     // {

//     // }
//     if(a == 2)
//     {
//         a= 0;
//     }
//     else
//     {
//         a = 1;
//         if(a == 1)
//         {
//             while(a == func())
//             {
                
//             }
//         }
//     }
//     a = 6;
// }
float asd()
{
    //return test;
    // func( a,  b);
    float a;
    // bool b;
    // b = true;
    //a = 5;
    return a;
    // fun();
}

int a;
float b;
bool c;

bool test(int a, float b, bool c)
{
    float test;
    bool aaa;
    // test = 5.0;
    // aaa = false;
    return aaa;
}

void tester(int v)
{
    // int as;
}

int ret(float h)
{
    return 0;
}

//ret(5.0); //func call outside block not allowed

int o;
// float o; //duplicate names of global variables not allowed
int main()
{
    // bool a;
    // int a;
    // int b;
    // int c;
    bool or;
    int or1;
    float or2;
    bool and;
    float eq;
    bool neq;
    bool le;
    bool lt; 
    bool ge;
    bool gt;
    float plus;
    float minus;
    float mod;
    float div;
    float mult;
    float unary;
    float not;

    float combo;
    // b = 10;
    // // a = -r;
    // a = --b; //in mini-c, this is double negate, in normal c, this is decrement (not in mini-c grammar)
    // // int b;
    // tester(-a);
    // // a = asd(b);
    // tester(o);
    // r = ret(10.2);
    // lineTwo(a,90.0);

    //testing assign 
    // a = true;
    // b = a;
    // c = a;

    //testing or

    // or = 0||0; //false
    // or = false || false; //false
    // or = 0.0||0.0; //false
    // or = 0 || true; //true
    // or = 34.0 || 0.0; //true
    or = 0.03 || 0.000; //true

    // or1 = or;
    // or2 = or;

    //testing and

    and = 0.00001 && 0.0;

    //testing eq

    eq = 234.4 == 234.4;

    //testing neq

    neq = 0.0 != 54.0;

    //testing le

    le = true <= false;

    //testing lt

    lt = true < false; 

    //testing ge

    ge = true >= false;

    //testing gt

     gt = true > false;

    //testing plus

    plus = true + false;

    //testing minus

    minus = true - false - false;

    //testing mult

    mult = true * true * false;

    //testing div

    div = 200.34 / 2; //div by 0
    div = false / true;

    //testing mod

    mod = 345 % 5;

    //test unary

    unary = -100.0;

    //test not

    not = !2324.23;

    //test combo

    combo = (5 + 4 * 89 / (124 % 66) * (54.0 != 340)) + ((6546.0 / 54.0) * -34 * (100/true)); //0xC11927C240000000
    combo = (5 + 4 * 89 / (124 % 66) * (54.0 != 340)) || ((6546.0 / 54.0) * -34 * (100/true)); //1.0


    // combo = ((5) + 4 * 3 % 3); //gives 5

    // float c;
    // float e;
    // int d;

    // combo = true + false;
    // combo = a = true / -1 + 10.0 + false; //gives 9
    // combo = c = e = d = 90.0;
    // combo = false || 1; 

    // combo = --10--20;
    // combo = ! -0.0; 


    return ret(true); //widen parameter size if its not a float

    //IR not generated for folling lines of code
    a = 100;
    return ret(90.0);
}