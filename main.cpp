#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cstdlib>
#include <ctime>
#include <sstream> 
#include <algorithm>
#include <windows.h>
#include <iomanip>
#include <iterator>

#define cimg_use_png
#include "CImg.h"
using namespace cimg_library;
#include "png.h"

using std::cout, std::cin, std::flush, std::ifstream, std::ofstream;

std::vector<unsigned int> split(const std::string& s, char delimiter) {
  std::vector<unsigned int> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter))
  {
    unsigned int tmp;
    std::stringstream ss;
    ss << std::hex << token;
    ss >> tmp;

    tokens.push_back(tmp);
  }
  return tokens;
}

void RGBAtoOBJ(const char* location) {
  CImg<uint8_t> image(location);
  cout << "Width: " << image.width() << std::endl;
  cout << "Height: " << image.height() << std::endl;

  std::string objname;
  cout << "Enter name output: ";
  cin >> objname;

  CreateDirectoryA(objname.c_str(), NULL); //fun windows bs :D
  std::string filepath = "./" + objname + "/" + objname;

  //handle obj
  ofstream object(filepath + ".obj");
  object << "mtllib " << objname << ".mtl\no " << objname << "\n\nvt 0.0 0.0\nvt 1.0 0.0\nvt 0.0 1.0\nvt 1.0 1.0\n\nvn  0.0  0.0  1.0\nvn  0.0  0.0 -1.0\nvn  0.0  1.0  0.0\nvn  0.0 -1.0  0.0\nvn  1.0  0.0  0.0\nvn -1.0  0.0  0.0\n";

  object << "\n#front\n";
  for(int h = 0, max_h = (image.height() + 1); h < max_h; h++) {
    for(int w = 0, max_w = (image.width() + 1); w < max_w; w++) {
      object << "v " << w << " " << h << " " << "1\n";
    }
  }

  object << "\n#back\n";
  for(int h = 0, max_h = (image.height() + 1); h < max_h; h++) {
    for(int w = 0, max_w = (image.width() + 1); w < max_w; w++) {
      object << "v " << w << " " << h << " " << "0\n";
    }
  }

  int back = (1 + image.width()) * (1 + image.height());
  std::set<uint32_t> mtl;
  for(uint32_t i = 0, max = image.width() * image.height(); i < max; i++) {
    if(image(i%image.width(), i/image.width(), 3) == 0) continue;

    uint32_t pixel = (image(i%image.width(), i/image.width(), 0) << 24) + (image(i%image.width(), i/image.width(), 1) << 16) + (image(i%image.width(), i/image.width(), 2) << 8) + image(i%image.width(), i/image.width(), 3);
    mtl.insert(pixel);
    object << "\ng " << "cube" << i << "\nusemtl " << std::setfill('0') << std::setw(8) << std::hex << pixel << std::dec << "\n";

    //left and right is based on you looking toward the face
    int front_bottom_left  = (i%image.width()) + 1 + ((image.height() - (i/image.width()) - 1) * (image.width()+1));
    int front_bottom_right = front_bottom_left + 1;
    int front_top_left     = front_bottom_left + image.width() + 1;
    int front_top_right    = front_top_left + 1;
    int back_bottom_right  = front_bottom_left + back;
    int back_bottom_left   = front_bottom_right + back;
    int back_top_right     = front_top_left + back;
    int back_top_left      = front_top_right + back;

    //front
    object << "f " << front_bottom_left  << "/1/1 " << front_top_right    << "/4/1 " << front_bottom_right << "/2/1\n";
    object << "f " << front_bottom_left  << "/1/1 " << front_top_left     << "/3/1 " << front_top_right    << "/4/1\n";
       
    //back   
    object << "f " << back_bottom_left   << "/1/2 " << back_top_right     << "/4/2 " << back_bottom_right  << "/2/2\n";
    object << "f " << back_bottom_left   << "/1/2 " << back_top_left      << "/3/2 " << back_top_right     << "/4/2\n";

    //left
    object << "f " << back_bottom_right  << "/1/3 " << front_top_left     << "/4/3 " << front_bottom_left  << "/2/3\n";
    object << "f " << back_bottom_right  << "/1/3 " << back_top_right     << "/3/3 " << front_top_left     << "/4/3\n";

    //right
    object << "f " << front_bottom_right << "/1/4 " << back_top_left      << "/4/4 " << back_bottom_left   << "/2/4\n";
    object << "f " << front_bottom_right << "/1/4 " << front_top_right    << "/3/4 " << back_top_left      << "/4/4\n";

    //top
    object << "f " << front_top_left     << "/1/5 " << back_top_left      << "/4/5 " << front_top_right    << "/2/5\n";
    object << "f " << front_top_left     << "/1/5 " << back_top_right     << "/3/5 " << back_top_left      << "/4/5\n";
 
    //back 
    object << "f " << back_bottom_right  << "/1/6 " << front_bottom_right << "/4/6 " << back_bottom_left   << "/2/6\n";
    object << "f " << back_bottom_right  << "/1/6 " << front_bottom_left  << "/3/6 " << front_bottom_right << "/4/6\n";
  }

  ofstream material(filepath + ".mtl");
  for(int i = 0; i < mtl.size(); i++) {
    auto pos = mtl.begin();
    std::advance(pos, i);
    if((*pos & 0xff) != 0) {
      double R = (double)((*pos >> 24) & 0xff) / 255;
      double G = (double)((*pos >> 16) & 0xff) / 255;
      double B = (double)((*pos >> 8)  & 0xff) / 255;
      material << "newmtl " << std::setfill('0') << std::setw(8) << std::hex << *pos << "\n  illum 0\n  Kd " << std::dec << std::setprecision(15) << R << " " << G << " " << B << "\n";
    }
  }
  
  material.close();
  object.close();
}

int main(int argc, char *argv[]) {
  std::string location;
  if(argc > 1) {
    location = argv[1];
  }
  else {
    cout << "Enter PNG you want to use: ";
    std::getline(cin, location, '\n');
  }
  RGBAtoOBJ(location.c_str());

  std::getchar();
}