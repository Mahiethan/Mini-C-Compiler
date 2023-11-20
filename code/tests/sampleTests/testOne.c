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

bool test(int a, int b, bool c)
{
    float test;
    bool aaa;
    // int aaa; //redefinition
    test = 5.0;
    aaa = false;
    test = a + b + c;
    //aaa = a; //a is detected but cannot be casted to bool
    // return 1; //adds appropriate casting for return
    return aaa;
}

int addition(int n, int m)
{
	int result;
	result = n + m;
    return result;
}

void tester(int v)
{
    // int as;
    return;
}

int ret(float h)
{
    return false;
}

//ret(5.0); //func call outside block not allowed

int o;
bool globool;
// float o; //duplicate names of global variables not allowed
int main()
{
    // bool a;
    // int a;
    // int b;
    // int c;
    // bool or;
    // int or1;
    // float or2;
    // bool and;
    // float eq;
    // bool neq;
    // bool le;
    // bool lt; 
    // bool ge;
    // bool gt;
    // float plus;
    // float minus;
    // float mod;
    // float div;
    // float mult;
    // float unary;
    // int not;

    float combo;
    int o;
    // float comboTwo;
    // float comboThree;

    // float comboThree; //redefiniton

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
    // or = 0.0 || 2; //true
    // or = 10 || false; //true

    // // // or1 = or;
    // // // or2 = or;

    // // //testing and

    // and = 0.00001 && 0.0; //false
    // and = false && true && true; //false

    // // // //testing eq

    // eq = 234.4 == 234.4; //true

    // // // //testing neq

    // neq = 54 != 54.0; //false

    // // // //testing le

    // le = true <= 10.0; //true

    // // // //testing lt

    // lt = true < false;  //false

    // // //testing ge

    // ge = 0.0 >= false; //true

    // // //testing gt

    //  gt = true > false;

    // // //testing plus

    // plus = true + false; //1.0

    // // //testing minus

    // minus = true - false - 10; //1 - 0 - 10 = -9

    // // //testing mult

    // mult = true * true * 92; //92.0
    // // mult = 10.0;

    // // mult = false * mult; //0

    // // // //testing div

    // // div = 200.34 / 0; //div by 0 not permitted
    // div = false / true; // 0.0

    // // //testing mod

    // // mod = 2132.0 % 0.1; //0x3FB99784A0000000 //clang -emit-llvm thinks these are doubles and gives error (fmod only works with floats not doubles)
    // mod = 123 % 3;
    // // mod = 345 % 0; //mod by 0 not permitted

    // // //test unary

    // unary = -100.0; //-100.0

    // unary = combo = -unary;

    // // //test not

    // not = -!0.0; //-1
    // not = -!1; //0

    // not = !unary + not;

    // // //test combo

    // combo = (5 + 4 * 89 / 124 % 66 * (54.0 != 340)) + (54.2 / 54.0) * -34 * 100/true + (0.0 && 50/50); //0xC0AA9B2F80000000 (due to lack of precision, testOne produced a double that truncates to a value very similar)
    // combo = ((5 + 4 * 89 / 124 % 66 * (54.0 != 340)) || (6546.0 / 54.0) * -34 * 100/true)/132.2; //0x3F7EEFC240000000
    // combo = 0 && (5 + 4 * 89 / 124 % 66 * (54.0 != 340)) + (6546.0 / 54.0) * -34 * 100/true + (0.0 && 50/50); //0.0
    // combo = true || (5 + 4 * 89 / 124 % 66 * (54.0 != 340)) + (6546.0 / 54.0) * -34 * 100/true + (0.0 && 50/50); //1.0
    // comboTwo = 0 && ((5 + 4 * 89 / 124 % 66 * (54.0 != 340)) + (6546.0 / 54.0) * -34 * 100/true + (0.0 && 50/50)); //0.0
    // comboThree = true || ((5 + 4 * 89 / 124 % 66 * (54.0 != 340)) + (6546.0 / 54.0) * -34 * 100/true + (0.0 && 50/50)); //1.0

    int ty; int flo; int A;
    ty = o;
    // combo = (ty == 1.0);

    // if(ty == (float) 1.1) //for testOne.ll, add (float) in front of 1.1
    // {
    //     float ty;
    //     ty = 10;
    //     o = 4; //global variable
    // }
    // else
    // {
    //     bool elseVal;
    //     elseVal = false;
    //     o = 10;
    // }

    // if(o == 10)
    // {
    //     ty = 1;
    //     // return ty; //doesn't generate if_end branch
    //     if(ty == 1)
    //     {
    //         o = 10;
    //         // return 1;
    //     }
    //     else
    //     {
    //         return 0;
    //     }
    // }
    // else
    // {
    //     return ty;
    //     ty = 100; //ignored
    // }

    // elseVal = true; //gives scope error

    //WORKS
    // if(ty == 100)
    // {
    //     while(ty == 100)
    //     {
    //         int flo;
    //         if(flo == 1)
    //         {
    //             return true;
    //         }
    //         // else
    //         // {
    //         //     flo = false;
    //         //     return flo;
    //         // }
    //         test = 999;
    //         func(10);
    //     }
    //     tester = 911;
    //     if(tester == 911)
    //     {

    //     }
    //     else
    //     {
    //         t = 0;
    //     }
    // }
    // else
    // {
    //     return false;
    // }

    //WORKS!!
    if(a == 10) //if_then
    {
        int hg_o;
        hg_o = 99;
        while(ty == 10) //while_cond
        {
            //while_body
            o = false;
            if(ty != 19)//if_then4
            {
                int hg;
                while(a == 10)//while_cond8
                {
                    //while_body9
                    return 0;
                    hg = a - 1;
                }
                //while_end
                hg = 0;
            }
            else//if_else5
            {
                ty = 10;
                hg_o = 0;
            }
            //if_end
        }
        //while_end13
        o = 10;
    }
    else //if_else
    {
        flo = 190;
    }
    //if_end14

    // tester(23.2); //should give semantic error (cannot narrow)
    tester(false); //valid - param widened
    o = -o;
    o = !globool;
    combo = true + 10.0;
    // {
    //     int combo;
    //     a = 5;
    //     b = a;
    //     // print_int(b);
    // }
    // o = !o; //cannot do narrow conversion from int to bool

    //WORKS
    // if(flo == 1) 
    // {
    //     if(a == 10)
    //     {

    //     }
    //     else
    //     {
    //         A = 1;
    //     }
    //     A = 0;
    // }
    // else
    // {
    //     a = 1;
    //     if(a == 10)
    //     {

    //     }
    //     else //else AST node only printed if it exists 
    //     {
    //         // A = 1;
    //     }
    //     a = 0;
    // }

    //WORKS!
    // if(a == 1)
    // {
    //     if(b != 1)
    //     {
    //     }
    //     else
    //     {
    //        c = 10;
    //     }
    //     tester = 999;
    // }
    // else
    // {
    //     tester = 1;
    // }

    //WORKS
    // if(a == 1) //if_then
    // {

    // }
    // else //if_else
    // {
    //     int a;
    //     a = 1;
    //     if(a == 1) //if_then2
    //     {
    //         a = !4; //same as a == 1
    //         if(a == 3) //if_then6
    //         {

    //         }
    //         return false; //if_end_0
    //         a = 3; //ignored
    //         if(a != 2) //ignored
    //         {
    //             ty = 9999;
    //         }
    //         else //ignored
    //         {

    //         }
    //     }
    //     else //if_else3
    //     {
    //         o = 10;
    //     }

    //     if(a == 10000)//if_end10
    //     {
    //         //if_then11
    //     }
    //     a = 999; //if_end_15
    // }
    //if_end16

    // while(a == 1)
    // {
    //     // return 0;
    //     o = 10;
    //     return 0;
    // }



    //aaa = 100; //can't refer to variables in other functions
    combo = o - -9; //should give 19.0
    combo = ((5) + 4 * false % 3); //gives 5.0

    // float c;
    // float e;
    // int d;

    // combo = true + false;
    // combo = a = true / -1 + 10.0 + false; //gives 9
    // combo = c = e = d = 90.0;
    // combo = false || 1; 

    // combo = --10--20;
    // combo = ! -0.0; 


    // return ret(true); //widen parameter size if its not a float
    // return 0.0; //gives error as expected
    // lineTwo(o,combo);

    return false; //will be widened to i32 0

    //IR not generated for folling lines of code
    a = 100;
    return ret(90.0);
}