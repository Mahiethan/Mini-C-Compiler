void call()
{

}
void firstFunc(int n, int m){int result;result = 3 + 4 * -!m;}
int secondFunc(float m){
    float result;
    //void call(); //cant call functions within functions
    if(result == 4.0)
    {
        result = 3 - 4 * -!m;
    }
    else
    {
        result = 7;
    }


    while(result == 7)
    {
        result = 7;
    }

    return 1;

    //causing infinite loop
    //not supposed to work - print error instead of infinitely looping
    // for(int i = 7; i < 10; i++)
    // {
    //     result = 7;
    // }

    // result = 10; //this and following not printed out in IR, but does appear in AST
    if(result == 10)
    {
        result = 11;
    }

}