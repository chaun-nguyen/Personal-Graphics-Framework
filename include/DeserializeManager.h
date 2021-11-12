#pragma once
#include "ManagerBase.h"
#include <string>
#include <vector>

class DeserializeManager : public ManagerBase<DeserializeManager>
{
public:
  DeserializeManager() = default;
  ~DeserializeManager() override;

  void Setup() override;
  void Update() override;

  std::vector<std::string> ReadSectionPath(const std::string& path);

  std::vector<std::string> SectionPaths;
  std::string preFixPath = { "./model/" };
};