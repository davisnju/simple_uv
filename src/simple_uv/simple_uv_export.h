
#ifndef SUV_EXPORT_H_23443546575
#define SUV_EXPORT_H_23443546575

#ifndef SUV_EXPORT
#  ifndef SIMPLE_UV_EXPORTS
#    define SUV_EXPORT __declspec(dllimport)
#  else
#    define SUV_EXPORT __declspec(dllexport)
#  endif  
#endif

#endif /* SUV_EXPORT_H_23443546575 */