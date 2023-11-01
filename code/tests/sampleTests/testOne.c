// works

//cant have func decl before extern
// int lineOne(int a, float b)
// {
//     return a;
// }
extern int lineTwo(int a, float b); 
extern int lineThree(int a, bool b); 
int testOne; //must have some decl after extern
int testTwo; //must have some decl after extern
void testBool(int f, float c)
{
    //variable declarations must go BEFORE any expressions
    int a; int b;
    a = 1;
    // a = b = 5 - (-!7) / 7 + 2 * 3;
    a = (5 + 3 / 3) * 3 % 3;
    // b = lineTwo(2+3/3-2,3%2);
    // float b;
    ;
    ;
    ;
    {

    }
    if(a == 1)
    {
        b = 2;
    }
    else
    {
        b = 3;
    }
    //b = 1;
    //int a = 1; //invalid
    return a;
}
void qr()
{
    int a;
    //bottom alone not allowed!!
    //int a = 1;
    a = 1;
}
void asd(int test)
{
    //return test;
}