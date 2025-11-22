#include <iostream>
#include <vector>
#include "RectangularMembrane.h"


int main(int argc, char** argv){
    std::string output_path;
    if (argc < 2){
        std::cout << "Usage: " << argv[0] << "/file/path/to/output.bin" << std::endl;
        return -1;
    }else{
        output_path = argv[1];
    }

    RectangularMembrane drum;

    drum.setInitialCondition();
    std::vector<float> audio_buffer;
    drum.Simulate(audio_buffer);

    //Write out the audio buffer to a file
    FILE *fp = fopen(output_path.c_str(), "wb");
    fwrite(audio_buffer.data(),1, audio_buffer.size() * sizeof(float), fp);
    fclose(fp);
}