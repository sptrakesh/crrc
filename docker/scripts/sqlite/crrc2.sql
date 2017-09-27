create table courses
(
  course_id integer primary key,
  identifier text not null,
  name text not null,
  credits text not null,
  description text,
  knowledge_unit integer default 0,
  department_id integer not null references departments (department_id) on delete cascade
);

create index idx_courses_department on courses (department_id);
