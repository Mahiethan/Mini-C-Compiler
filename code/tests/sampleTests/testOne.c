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
int testBool(int f, float c)
{
    //variable declarations must go BEFORE any expressions
    int a; int b;
    a = 1;
    a = b = 5 - 7 / 7 + 2 * 3;
    // float b;
    ;
    ;
    ;
    //b = 1;
    //int a = 1; //invalid
    return a;
}