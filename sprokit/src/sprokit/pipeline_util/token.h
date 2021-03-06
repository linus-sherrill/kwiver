// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Interface to lexical token.
 */

#ifndef SPROKIT_PIPELINE_TOKEN_H
#define SPROKIT_PIPELINE_TOKEN_H

#include<sprokit/pipeline_util/sprokit_pipeline_util_export.h>

#include <vital/util/source_location.h>

#include <iostream>
#include <string>
#include <memory>

namespace sprokit {

/**
* These are all the token type codes
* Start token values above character values
*/

enum token_type_t {
  TK_FIRST = 1024, // must be first

  TK_NONE,       // self defining token, such as a character

  TK_IDENTIFIER,     // word

  TK_CLUSTER_DESC, // '--' description marker for cluster defs

  TK_LOCAL_ASSIGN, // :=
  TK_ASSIGN,       // =
  TK_COLON,       // ^: start of sprokit style config

  // Keyword tokens
  TK_PROCESS,
  TK_STATIC,
  TK_ATTRIBUTE, // for all attribute words
  TK_CONNECT,
  TK_FROM,
  TK_TO,
  TK_RELATIVE_PATH,

  TK_BLOCK,
  TK_ENDBLOCK,
  TK_CONFIG,

  TK_CLUSTER,
  TK_IMAP,
  TK_OMAP,

  TK_DOUBLE_COLON,

  // These are last in the list - order counts
  TK_WHITESPACE,
  TK_EOL,
  TK_EOF,        // End Of File

  TK_LAST       // highest token value
};

// ----------------------------------------------------------------
/**
 * @brief Lexical token
 *
 * This class represents a lexical token.
 */

class SPROKIT_PIPELINE_UTIL_EXPORT token
{
public:
  // -- CONSTRUCTORS --
  token();
  token( int code );
  token( int type, const std::string& s );
  virtual ~token();

  // -- ACCESSORS --
  const std::string& text() const { return m_text; }

  int token_type() const { return m_token_type; }
  int token_value() const;
  const kwiver::vital::source_location& get_location() const { return m_srcLocation; }

  virtual std::ostream& format( std::ostream& str ) const;
  static const char* token_name( int tk );

  // -- MANIPULATORS --
  void text( const std::string& str ) { m_text = str; }
  void set_location( const kwiver::vital::source_location& s ) { m_srcLocation = s; }

private:
  int m_token_type;           // token type code
  std::string m_text;         // String associated with token

  // where token was discovered
  kwiver::vital::source_location m_srcLocation;

  // SourceLineRef m_defLine;  // line where discovered
};

inline std::ostream&
operator<<( std::ostream& str, const token& obj )
{ return obj.format( str ); }

typedef std::shared_ptr< token > token_sptr;

} // end namespace

#endif /* SPROKIT_PIPELINE_TOKEN_H */
