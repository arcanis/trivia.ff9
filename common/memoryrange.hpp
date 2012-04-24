#pragma once

class MemoryRange {

public:
  enum SeekSource {
	SeekSet,
	SeekCur,
	SeekEnd
  };

public:
  MemoryRange(char const * begin, char const * end);

public:
  MemoryRange & seek(SeekSource whence, long offset);

  MemoryRange & crop(unsigned long offset, unsigned long size);

public:
  char const * begin(void) const {
	return m_begin;
  }

  char const * end(void) const {
	return m_end;
  }

public:
  char const * current(void) const {
	return m_current;
  }

  char const * & current(void) {
	return m_current;
  }

private:
  char const * m_begin;
  char const * m_current;
  char const * m_end;

};
