#include "parser/glsl_parser.hpp"

GLSLParser::GLSLParser()
{
}

bool GLSLParser::parse(QByteArray data) {
  m_pp.reset();
  return m_pp.parse(data);
}