project('websock', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])
# Fix for old meson where cpp_std=c++20 is not available
add_project_arguments('-std=c++20', language : 'cpp')

libsocks_proj = subproject('socks')
libsocks_dep = libsocks_proj.get_variable('libsocks_dep')

executable('websock',
           'Websocket.cpp',
           'sha1/sha1.cpp',
           'main.cpp',
           dependencies : [ libsocks_dep, dependency('threads') ],
           install : true)

executable('websock2',
           'Websocket.cpp',
           'sha1/sha1.cpp',
           'main2.cpp',
           dependencies : [ libsocks_dep, dependency('threads') ],
           install : true)
