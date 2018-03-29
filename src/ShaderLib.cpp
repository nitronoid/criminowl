#include "ShaderLib.h"
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <string>
#include <fstream>
#include <streambuf>
#include <regex>

std::string ShaderLib::loadShaderProg(const QString &_jsonFileName)
{
  auto toStr = [](const auto& val){ return val.toString(); };
  // Read in raw file
  QFile file(_jsonFileName);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QByteArray rawData = file.readAll();
  // Parse document
  QJsonDocument doc(QJsonDocument::fromJson(rawData));
  // Get the json object to view
  QJsonObject shaderParts = doc.object();

  // Get a string out from the json
  std::string shaderName = shaderParts["Name"].toString().toStdString();

  static constexpr std::array<const char*, 5> shaderNames = {
    {"Vertex", "Fragment", "Geometry", "TessellationControl", "TessellationEvaluation"}
  };
  std::array<QString, 5> shaderPaths;

  for (auto shader : {VERTEX, FRAGMENT, GEOMETRY, TESSCONTROL, TESSEVAL})
  {
    auto& name = shaderNames[shader];
    shaderPaths[shader] = shaderParts.contains(name) ? toStr(shaderParts[name]) : "";
  }

  // Load the shader if we haven't already
  if (!m_shaderPrograms.count(shaderName))
    createShader(shaderName, shaderPaths);

  return shaderName;
}

void ShaderLib::createShader(const std::string &_name, const std::array<QString, 5> &_shaderPaths)
{
  QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

  using shdr = QOpenGLShader;
  static constexpr shdr::ShaderType qShaders[] = {
    shdr::Vertex, shdr::Fragment, shdr::Geometry, shdr::TessellationControl, shdr::TessellationEvaluation
  };
  for (auto shader : {VERTEX, FRAGMENT, GEOMETRY, TESSCONTROL, TESSEVAL})
  {
    auto path = _shaderPaths[shader];
    if (path == "") continue;
    auto stdPath = path.toStdString();
    if (!m_shaderParts.count(stdPath))
    {
      QOpenGLShader* shad = new QOpenGLShader(qShaders[shader]);
      std::string shaderString = loadFileToString(stdPath);
      parseIncludes(shaderString);

      shad->compileSourceCode(shaderString.c_str());
      m_shaderParts[stdPath].reset(shad);
    }
    program->addShader(m_shaderParts[stdPath].get());
  }
  program->link();
  m_shaderPrograms[_name].reset(program);
}

namespace std
{

template<class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT> regex_replace(BidirIt _first, BidirIt _last, const std::basic_regex<CharT,Traits>& _re, UnaryFunction _f)
{
  std::basic_string<CharT> s;

  typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;
  auto endOfLastMatch = _first;

  auto callback = [&](const std::match_results<BidirIt>& match)
  {
    auto positionOfThisMatch = match.position(0);
    auto diff = positionOfThisMatch - positionOfLastMatch;

    auto startOfThisMatch = endOfLastMatch;
    std::advance(startOfThisMatch, diff);

    s.append(endOfLastMatch, startOfThisMatch);
    s.append(_f(match));

    auto lengthOfMatch = match.length(0);

    positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

    endOfLastMatch = startOfThisMatch;
    std::advance(endOfLastMatch, lengthOfMatch);
  };

  std::regex_iterator<BidirIt> begin(_first, _last, _re), end;
  std::for_each(begin, end, callback);

  s.append(endOfLastMatch, _last);

  return s;
}

template<class Traits, class CharT, class UnaryFunction>
std::string regex_replace(const std::string& s, const std::basic_regex<CharT,Traits>& re, UnaryFunction f)
{
  return regex_replace(s.cbegin(), s.cend(), re, f);
}

} // namespace std

std::string ShaderLib::loadFileToString(const std::string &_path)
{
  std::string ret;
  std::ifstream shaderFileStream(_path);
  shaderFileStream.seekg(0, std::ios::end);
  ret.reserve(shaderFileStream.tellg());
  shaderFileStream.seekg(0, std::ios::beg);

  ret.assign((std::istreambuf_iterator<char>(shaderFileStream)), std::istreambuf_iterator<char>());
  return ret;
}

void ShaderLib::parseIncludes(std::string &io_shaderString)
{
  std::regex matcher(R"(#{1}include\ +(\"|\<)[a-zA-Z][a-zA-Z0-9_\/]+\.(h|glsl)(\"|\>))");
  io_shaderString = std::regex_replace(io_shaderString, matcher, [this](const std::smatch& _m)
  {
    auto str = _m.str();
    auto begin = str.find('"') + 1;
    auto end = str.find_last_of('"');
    return loadFileToString(std::string(str.begin() + begin, str.begin() + end));
  }
  );
}

void ShaderLib::useShader(const std::string& _name)
{
  m_currentShader = m_shaderPrograms[_name].get();
  m_currentShader->bind();
}

QOpenGLShaderProgram *ShaderLib::getShader(const std::string& _name)
{
  return m_shaderPrograms[_name].get();
}

QOpenGLShaderProgram* ShaderLib::getCurrentShader()
{
  return m_currentShader;
}

