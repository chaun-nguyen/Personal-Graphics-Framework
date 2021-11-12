#include "DeserializeManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

DeserializeManager::~DeserializeManager()
{
}

void DeserializeManager::Setup()
{
  for (int i = 1; i <= 2; ++i)
  {
    std::stringstream ss;
    ss << preFixPath;
    ss << "Animation";
    ss << i;
    ss << ".txt";
    SectionPaths.push_back(ss.str());
  }
}

void DeserializeManager::Update()
{
}

std::vector<std::string> DeserializeManager::ReadSectionPath(const std::string& path)
{
  std::vector<std::string> parts;
  std::ifstream file(path, std::ifstream::in);

  if (file.is_open())
  {
    std::string line;
    while (std::getline(file, line))
    {
      std::stringstream ss;
      ss << preFixPath;
      ss << line;
      parts.push_back(ss.str());
    }
    file.close();
  }
  else
  {
    std::cerr << "Make Sure you have a right filepath convention to load model. Please refer to README.txt" << std::endl;
    throw std::runtime_error("Make Sure you have a right filepath convention");
  }

  return parts;
}
