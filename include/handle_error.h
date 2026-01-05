#pragma once

enum severity_type {
	SEVERITY_FATAL,
	SEVERITY_WARNING,
};

void handle_error(const char *message, const enum severity_type severity);
