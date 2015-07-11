#ifndef COMPAT_H
	#ifdef _WIN32
		// Careful, return semantics are different between snprintf and _snprintf
		#ifndef snprintf
			#define snprintf _snprintf_s
		#endif
	#endif

#endif
