#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "base64.h"

using namespace std;

int main()
{
    cout << "- Binary file to Base64 text - " << endl;

    //get input
    string s_input;
    while( true )
    {
        cout<<"\nType name of file to be converted: ";
        cin>>s_input;
        //test file
        ifstream file_test( s_input.c_str(),ios_base::binary );
        if(file_test)
        {
            //file OK
            file_test.close();
            break;
        }
        else
        {
            //file not ok
            cout<<"\nERROR: Could not find that file!\n";
        }
    }

    //file OK
    ifstream file_binary_input( s_input.c_str(),ios_base::binary );
    if(!file_binary_input)
    {
        cout<<"ERROR: Could not read binary file!\n";
        system("pause");
        return 1;
    }
    //read binary data
    string s_binary_data;
    while( file_binary_input.good() )
    {
        unsigned char c=file_binary_input.get();
        if( file_binary_input.good() ) s_binary_data.append(1,c);
    }
    file_binary_input.close();

    //encode to base64
    string s_code=base64_encode( (unsigned char*)s_binary_data.c_str(), s_binary_data.length() );

    //output code to file
    ofstream file_text_output("code.txt");
    if(!file_text_output)
    {
        cout<<"ERROR: Could not create text file!\n";
        system("pause");
        return 1;
    }
    //file OK
    //file_text_output<<s_code;
    //output with parsing
    int chars_per_line=90;
    int current_char_on_line=0;
    file_text_output<<"//";
    file_text_output<<s_input;
    file_text_output<<"\n\"";
    for(int c=0;c<(int)s_code.length();c++)
    {
        file_text_output<<s_code[c];
        current_char_on_line++;
        if(current_char_on_line>chars_per_line)
        {
            //end this line
            file_text_output<<"\"";
            //test if should start new line
            if( c!=(int)s_code.length()-1 ) file_text_output<<"\n\"";
            //reset counter
            current_char_on_line=0;
        }

    }
    //end final line
    file_text_output<<"\";";
    file_text_output.close();

    cout<<"\nBinary data has been converted to text in \"code.txt\"\n";
    system("pause");

    return 0;
}
