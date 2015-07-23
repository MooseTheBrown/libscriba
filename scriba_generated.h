// automatically generated, do not modify

#include "flatbuffers/flatbuffers.h"

namespace scriba {

enum {
  EventType_MEETING = 0,
  EventType_CALL = 1,
  EventType_TASK = 2,
};

inline const char **EnumNamesEventType() {
  static const char *names[] = { "MEETING", "CALL", "TASK", nullptr };
  return names;
}

inline const char *EnumNameEventType(int e) { return EnumNamesEventType()[e]; }

enum {
  EventState_SCHEDULED = 0,
  EventState_COMPLETED = 1,
  EventState_CANCELED = 2,
};

inline const char **EnumNamesEventState() {
  static const char *names[] = { "SCHEDULED", "COMPLETED", "CANCELED", nullptr };
  return names;
}

inline const char *EnumNameEventState(int e) { return EnumNamesEventState()[e]; }

enum {
  ProjectState_INITIAL = 0,
  ProjectState_CLIENT_INFORMED = 1,
  ProjectState_CLIENT_RESPONSE = 2,
  ProjectState_OFFER = 3,
  ProjectState_REJECTED = 4,
  ProjectState_CONTRACT_SIGNED = 5,
  ProjectState_EXECUTION = 6,
  ProjectState_PAYMENT = 7,
};

inline const char **EnumNamesProjectState() {
  static const char *names[] = { "INITIAL", "CLIENT_INFORMED", "CLIENT_RESPONSE", "OFFER", "REJECTED", "CONTRACT_SIGNED", "EXECUTION", "PAYMENT", nullptr };
  return names;
}

inline const char *EnumNameProjectState(int e) { return EnumNamesProjectState()[e]; }

enum {
  Currency_RUB = 0,
  Currency_USD = 1,
  Currency_EUR = 2,
};

inline const char **EnumNamesCurrency() {
  static const char *names[] = { "RUB", "USD", "EUR", nullptr };
  return names;
}

inline const char *EnumNameCurrency(int e) { return EnumNamesCurrency()[e]; }

struct ID;
struct Company;
struct Event;
struct POC;
struct Project;
struct Entries;

MANUALLY_ALIGNED_STRUCT(8) ID {
 private:
  uint64_t high_;
  uint64_t low_;

 public:
  ID(uint64_t high, uint64_t low)
    : high_(flatbuffers::EndianScalar(high)), low_(flatbuffers::EndianScalar(low)) {}

  uint64_t high() const { return flatbuffers::EndianScalar(high_); }
  uint64_t low() const { return flatbuffers::EndianScalar(low_); }
};
STRUCT_END(ID, 16);

struct Company : private flatbuffers::Table {
  const ID *id() const { return GetStruct<const ID *>(4); }
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(6); }
  const flatbuffers::String *jur_name() const { return GetPointer<const flatbuffers::String *>(8); }
  const flatbuffers::String *address() const { return GetPointer<const flatbuffers::String *>(10); }
  const flatbuffers::String *inn() const { return GetPointer<const flatbuffers::String *>(12); }
  const flatbuffers::String *phonenum() const { return GetPointer<const flatbuffers::String *>(14); }
  const flatbuffers::String *email() const { return GetPointer<const flatbuffers::String *>(16); }
};

struct CompanyBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(const ID *id) { fbb_.AddStruct(4, id); }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(6, name); }
  void add_jur_name(flatbuffers::Offset<flatbuffers::String> jur_name) { fbb_.AddOffset(8, jur_name); }
  void add_address(flatbuffers::Offset<flatbuffers::String> address) { fbb_.AddOffset(10, address); }
  void add_inn(flatbuffers::Offset<flatbuffers::String> inn) { fbb_.AddOffset(12, inn); }
  void add_phonenum(flatbuffers::Offset<flatbuffers::String> phonenum) { fbb_.AddOffset(14, phonenum); }
  void add_email(flatbuffers::Offset<flatbuffers::String> email) { fbb_.AddOffset(16, email); }
  CompanyBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<Company> Finish() { return flatbuffers::Offset<Company>(fbb_.EndTable(start_, 7)); }
};

inline flatbuffers::Offset<Company> CreateCompany(flatbuffers::FlatBufferBuilder &_fbb, const ID *id, flatbuffers::Offset<flatbuffers::String> name, flatbuffers::Offset<flatbuffers::String> jur_name, flatbuffers::Offset<flatbuffers::String> address, flatbuffers::Offset<flatbuffers::String> inn, flatbuffers::Offset<flatbuffers::String> phonenum, flatbuffers::Offset<flatbuffers::String> email) {
  CompanyBuilder builder_(_fbb);
  builder_.add_email(email);
  builder_.add_phonenum(phonenum);
  builder_.add_inn(inn);
  builder_.add_address(address);
  builder_.add_jur_name(jur_name);
  builder_.add_name(name);
  builder_.add_id(id);
  return builder_.Finish();
}

struct Event : private flatbuffers::Table {
  const ID *id() const { return GetStruct<const ID *>(4); }
  const flatbuffers::String *descr() const { return GetPointer<const flatbuffers::String *>(6); }
  const ID *company_id() const { return GetStruct<const ID *>(8); }
  const ID *project_id() const { return GetStruct<const ID *>(10); }
  const ID *poc_id() const { return GetStruct<const ID *>(12); }
  int8_t type() const { return GetField<int8_t>(14, 0); }
  const flatbuffers::String *outcome() const { return GetPointer<const flatbuffers::String *>(16); }
  int64_t timestamp() const { return GetField<int64_t>(18, 0); }
  int8_t state() const { return GetField<int8_t>(20, 0); }
};

struct EventBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(const ID *id) { fbb_.AddStruct(4, id); }
  void add_descr(flatbuffers::Offset<flatbuffers::String> descr) { fbb_.AddOffset(6, descr); }
  void add_company_id(const ID *company_id) { fbb_.AddStruct(8, company_id); }
  void add_project_id(const ID *project_id) { fbb_.AddStruct(10, project_id); }
  void add_poc_id(const ID *poc_id) { fbb_.AddStruct(12, poc_id); }
  void add_type(int8_t type) { fbb_.AddElement<int8_t>(14, type, 0); }
  void add_outcome(flatbuffers::Offset<flatbuffers::String> outcome) { fbb_.AddOffset(16, outcome); }
  void add_timestamp(int64_t timestamp) { fbb_.AddElement<int64_t>(18, timestamp, 0); }
  void add_state(int8_t state) { fbb_.AddElement<int8_t>(20, state, 0); }
  EventBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<Event> Finish() { return flatbuffers::Offset<Event>(fbb_.EndTable(start_, 9)); }
};

inline flatbuffers::Offset<Event> CreateEvent(flatbuffers::FlatBufferBuilder &_fbb, const ID *id, flatbuffers::Offset<flatbuffers::String> descr, const ID *company_id, const ID *project_id, const ID *poc_id, int8_t type, flatbuffers::Offset<flatbuffers::String> outcome, int64_t timestamp, int8_t state) {
  EventBuilder builder_(_fbb);
  builder_.add_timestamp(timestamp);
  builder_.add_outcome(outcome);
  builder_.add_poc_id(poc_id);
  builder_.add_project_id(project_id);
  builder_.add_company_id(company_id);
  builder_.add_descr(descr);
  builder_.add_id(id);
  builder_.add_state(state);
  builder_.add_type(type);
  return builder_.Finish();
}

struct POC : private flatbuffers::Table {
  const ID *id() const { return GetStruct<const ID *>(4); }
  const flatbuffers::String *firstname() const { return GetPointer<const flatbuffers::String *>(6); }
  const flatbuffers::String *secondname() const { return GetPointer<const flatbuffers::String *>(8); }
  const flatbuffers::String *lastname() const { return GetPointer<const flatbuffers::String *>(10); }
  const flatbuffers::String *mobilenum() const { return GetPointer<const flatbuffers::String *>(12); }
  const flatbuffers::String *phonenum() const { return GetPointer<const flatbuffers::String *>(14); }
  const flatbuffers::String *email() const { return GetPointer<const flatbuffers::String *>(16); }
  const flatbuffers::String *position() const { return GetPointer<const flatbuffers::String *>(18); }
  const ID *company_id() const { return GetStruct<const ID *>(20); }
};

struct POCBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(const ID *id) { fbb_.AddStruct(4, id); }
  void add_firstname(flatbuffers::Offset<flatbuffers::String> firstname) { fbb_.AddOffset(6, firstname); }
  void add_secondname(flatbuffers::Offset<flatbuffers::String> secondname) { fbb_.AddOffset(8, secondname); }
  void add_lastname(flatbuffers::Offset<flatbuffers::String> lastname) { fbb_.AddOffset(10, lastname); }
  void add_mobilenum(flatbuffers::Offset<flatbuffers::String> mobilenum) { fbb_.AddOffset(12, mobilenum); }
  void add_phonenum(flatbuffers::Offset<flatbuffers::String> phonenum) { fbb_.AddOffset(14, phonenum); }
  void add_email(flatbuffers::Offset<flatbuffers::String> email) { fbb_.AddOffset(16, email); }
  void add_position(flatbuffers::Offset<flatbuffers::String> position) { fbb_.AddOffset(18, position); }
  void add_company_id(const ID *company_id) { fbb_.AddStruct(20, company_id); }
  POCBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<POC> Finish() { return flatbuffers::Offset<POC>(fbb_.EndTable(start_, 9)); }
};

inline flatbuffers::Offset<POC> CreatePOC(flatbuffers::FlatBufferBuilder &_fbb, const ID *id, flatbuffers::Offset<flatbuffers::String> firstname, flatbuffers::Offset<flatbuffers::String> secondname, flatbuffers::Offset<flatbuffers::String> lastname, flatbuffers::Offset<flatbuffers::String> mobilenum, flatbuffers::Offset<flatbuffers::String> phonenum, flatbuffers::Offset<flatbuffers::String> email, flatbuffers::Offset<flatbuffers::String> position, const ID *company_id) {
  POCBuilder builder_(_fbb);
  builder_.add_company_id(company_id);
  builder_.add_position(position);
  builder_.add_email(email);
  builder_.add_phonenum(phonenum);
  builder_.add_mobilenum(mobilenum);
  builder_.add_lastname(lastname);
  builder_.add_secondname(secondname);
  builder_.add_firstname(firstname);
  builder_.add_id(id);
  return builder_.Finish();
}

struct Project : private flatbuffers::Table {
  const ID *id() const { return GetStruct<const ID *>(4); }
  const flatbuffers::String *title() const { return GetPointer<const flatbuffers::String *>(6); }
  const flatbuffers::String *descr() const { return GetPointer<const flatbuffers::String *>(8); }
  const ID *company_id() const { return GetStruct<const ID *>(10); }
  int8_t state() const { return GetField<int8_t>(12, 0); }
  int8_t currency() const { return GetField<int8_t>(14, 0); }
  uint64_t cost() const { return GetField<uint64_t>(16, 0); }
};

struct ProjectBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(const ID *id) { fbb_.AddStruct(4, id); }
  void add_title(flatbuffers::Offset<flatbuffers::String> title) { fbb_.AddOffset(6, title); }
  void add_descr(flatbuffers::Offset<flatbuffers::String> descr) { fbb_.AddOffset(8, descr); }
  void add_company_id(const ID *company_id) { fbb_.AddStruct(10, company_id); }
  void add_state(int8_t state) { fbb_.AddElement<int8_t>(12, state, 0); }
  void add_currency(int8_t currency) { fbb_.AddElement<int8_t>(14, currency, 0); }
  void add_cost(uint64_t cost) { fbb_.AddElement<uint64_t>(16, cost, 0); }
  ProjectBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<Project> Finish() { return flatbuffers::Offset<Project>(fbb_.EndTable(start_, 7)); }
};

inline flatbuffers::Offset<Project> CreateProject(flatbuffers::FlatBufferBuilder &_fbb, const ID *id, flatbuffers::Offset<flatbuffers::String> title, flatbuffers::Offset<flatbuffers::String> descr, const ID *company_id, int8_t state, int8_t currency, uint64_t cost) {
  ProjectBuilder builder_(_fbb);
  builder_.add_cost(cost);
  builder_.add_company_id(company_id);
  builder_.add_descr(descr);
  builder_.add_title(title);
  builder_.add_id(id);
  builder_.add_currency(currency);
  builder_.add_state(state);
  return builder_.Finish();
}

struct Entries : private flatbuffers::Table {
  const flatbuffers::Vector<flatbuffers::Offset<Company>> *companies() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Company>> *>(4); }
  const flatbuffers::Vector<flatbuffers::Offset<Event>> *events() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Event>> *>(6); }
  const flatbuffers::Vector<flatbuffers::Offset<POC>> *people() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<POC>> *>(8); }
  const flatbuffers::Vector<flatbuffers::Offset<Project>> *projects() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Project>> *>(10); }
};

struct EntriesBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_companies(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Company>>> companies) { fbb_.AddOffset(4, companies); }
  void add_events(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Event>>> events) { fbb_.AddOffset(6, events); }
  void add_people(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<POC>>> people) { fbb_.AddOffset(8, people); }
  void add_projects(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Project>>> projects) { fbb_.AddOffset(10, projects); }
  EntriesBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  flatbuffers::Offset<Entries> Finish() { return flatbuffers::Offset<Entries>(fbb_.EndTable(start_, 4)); }
};

inline flatbuffers::Offset<Entries> CreateEntries(flatbuffers::FlatBufferBuilder &_fbb, flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Company>>> companies, flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Event>>> events, flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<POC>>> people, flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Project>>> projects) {
  EntriesBuilder builder_(_fbb);
  builder_.add_projects(projects);
  builder_.add_people(people);
  builder_.add_events(events);
  builder_.add_companies(companies);
  return builder_.Finish();
}

inline const Entries *GetEntries(const void *buf) { return flatbuffers::GetRoot<Entries>(buf); }

}; // namespace scriba
