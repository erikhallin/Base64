#include <iostream>
#include <fstream>
#include <string>
#include "base64.h"
#include "sound.h"

using namespace std;

int main()
{
    cout << "START" << endl;

    /*//read encoded data from file to string -OK-
    ifstream code_input("code.txt");
    if(code_input==0)
    {
        cout<<"ERROR 1\n";
        return 1;
    }
    string line;
    string s_code;
    while( getline(code_input,line) )
    {
        s_code.append(line);
    }
    code_input.close();

    //decode that
    string s_decode=base64_decode( s_code );*/




    //ALT Get binary file -OK-
    ifstream is_binary_file("dong.wav",ios_base::binary);
    string s_binary_data;
    while( is_binary_file.good() )
    {
        unsigned char c=is_binary_file.get();
        if( is_binary_file.good() ) s_binary_data.append(1,c);
    }
    is_binary_file.close();

    //encode -OK-
    string s_code=base64_encode( (unsigned char*)s_binary_data.c_str(), s_binary_data.length() );

    //print encoded data -OK-
    ofstream of_encoded_data("encoded_data.txt");
    of_encoded_data<<s_code;
    of_encoded_data.close();

    //decode -OK-
    string s_decode=base64_decode( s_code );

    //create binary file from binary data -OK-
    ofstream binary_output("from_binary_data.wav",ios_base::binary);
    binary_output<<s_binary_data;
    binary_output.close();

    //create binary file from decoded string
    ofstream decoded_output("from_decoded_string.wav",ios_base::binary);
    decoded_output<<s_decode;
    decoded_output.close();

    //load sound
    cout<<"Playing sound\n";
    sound Sound;
    //load sound
    Sound.load_WAVE_from_string(wav_dong,s_decode);
    //Sound.load_WAVE_from_file(wav_dong,"dong.wav");
    //play sound
    Sound.playSimpleSound(wav_dong,1.0);
    system("pause");

    cout<<"DONE\n";

    return 0;
}
