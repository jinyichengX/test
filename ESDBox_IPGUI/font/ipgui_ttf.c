
// #define ipgui_file_suffix_check

int ipgui_file_suffix_check(const char * suffix, const char * filepath)
{
    int i = 0;

    while( filepath[i++] ){;}
    
    if( i > 4 )
    {
        if(( filepath[ i ] == 't' ) && \
           ( filepath[ i - 1 ] == 't' ) && \
           ( filepath[ i - 2 ] == 'f' ) && \
           ( filepath[ i - 3 ] == '.' ))
        {
            return 1;
        }
    }

    return 0;
}