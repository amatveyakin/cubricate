#ifndef TIME_HPP
#define TIME_HPP


class Time {
public:
  typedef unsigned int       MsecType;
  typedef unsigned long long UsecType;

  Time() { }
  ~Time() { }

  UsecType usec() const                     { return m_msec * 1000; }
  MsecType msec() const                     { return m_msec; }
  double   sec() const                      { return m_msec / 1000.; }

  void setUsec (UsecType usec)              { m_msec = usec / 1000; }
  void setMsec (MsecType msec)              { m_msec = msec; }
  void setSec (double sec)                  { m_msec = sec * 1000.; }

  Time operator+ (Time t) const             { return Time (m_msec + t.m_msec); }
  Time operator- (Time t) const             { return Time (m_msec - t.m_msec); }
  double operator/ (Time t) const           { return double (m_msec) / double (t.m_msec); }

  template <typename T>
  Time operator* (T coeff) const            { return Time (m_msec * coeff); }
  template <typename T>
  Time operator/ (T coeff) const            { return Time (m_msec / coeff); }

  bool operator< (Time t) const             { return m_msec < t.m_msec; }
  bool operator> (Time t) const             { return m_msec > t.m_msec; }
  bool operator<= (Time t) const            { return m_msec >= t.m_msec; }
  bool operator>= (Time t) const            { return m_msec >= t.m_msec; }

  static Time fromUsec (UsecType usec)      { return Time (usec / 1000); }
  static Time fromMsec (MsecType msec)      { return Time (msec); }
  static Time fromSec (double sec)          { return Time (sec * 1000.); }

private:
  Time (MsecType msec)                      { m_msec = msec; }

  MsecType m_msec;
};


#endif
