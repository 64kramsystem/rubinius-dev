/* Used by the message lookup logic. The requester fills in the
 * top fields with what they want, and the bottom fields are filled
 * in by the lookup machinery */
struct message {
  STATE;
  cpu c;
  OBJECT name;
  OBJECT recv;
  OBJECT klass;
  OBJECT block;
  int args;
  int priv;
  int stack;
  OBJECT send_site;
  OBJECT splat;

  OBJECT module;
  OBJECT method;
  int missing;
};
