/*
 * linux inotify test.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/inotify.h>

#define DEVICE_PATH "notify_dir"

typedef struct {
	int value;
	char *str;
} event_map_t;

static event_map_t event_map[] = {
	{IN_ACCESS,	"IN_ACCESS"},
	{IN_MODIFY, "IN_MODIFY"},
	{IN_ATTRIB, "IN_ATTRIB"},
	{IN_CLOSE_WRITE, "IN_CLOSE_WRITE"},
	{IN_CLOSE_NOWRITE, "IN_CLOSE_NOWRITE"},
	{IN_OPEN, "IN_OPEN"},
	{IN_MOVED_FROM, "IN_MOVED_FROM"},
	{IN_MOVED_TO, "IN_MOVED_TO"},
	{IN_CREATE, "IN_CREATE"},
	{IN_DELETE, "IN_DELETE"},
	{IN_DELETE_SELF, "IN_DELETE_SELF"},
	{IN_MOVE_SELF, "IN_MOVE_SELF"},
	{IN_UNMOUNT, "IN_UNMOUNT"},
	{IN_Q_OVERFLOW, "IN_Q_OVERFLOW"},
	{IN_IGNORED, "IN_IGNORED"},
	{IN_CLOSE, "IN_CLOSE"},
	{IN_MOVE, "IN_MOVE"},
	{IN_ONLYDIR, "IN_ONLYDIR"},
	{IN_DONT_FOLLOW, "IN_DONT_FOLLOW"},
	{IN_EXCL_UNLINK, "IN_EXCL_UNLINK"},
	{IN_MASK_ADD, "IN_MASK_ADD"},
	//{IN_ISDIR, "IN_ISDIR"},
	{IN_ONESHOT, "IN_ONESHOT"},
	{0, NULL}
};

int get_notify(int fd)
{
	int res = 0;
	uint8_t is_dir = 0;

	char event_buf[512];
	int event_size;
	int event_pos = 0;
	struct inotify_event *event;
	event_map_t *p = NULL;

	res = read(fd, event_buf, sizeof(event_buf));
	if(res < (int)sizeof(*event)) {
		if(errno == EINTR)
			return 0;

		printf("could not get event, %s\n", strerror(errno));
		return -1;
	}

	while(res >= (int)sizeof(*event)) {
		event = (struct inotify_event *)(event_buf + event_pos);
		if(event->len) {
			//printf("%d: %08x \"%s\"\n", event->wd, event->mask, event->len ? event->name : "");

			if(event->mask & IN_ISDIR)
				is_dir = 1;
			else
				is_dir = 0;

			for(p = &event_map[0]; p->value != 0; p++) {
				if(event->mask & p->value) {
					printf("wd=%d %-4s %-18s PATH=\"%s/%s\"\n", event->wd, is_dir ? "DIR" : "FILE",
									p->str, DEVICE_PATH, event->name);
				}
			}
		}
		event_size = sizeof(*event) + event->len;
		res -= event_size;
		event_pos += event_size;
	}

	return 0;
}

int main()
{
	int result = 0;
	int m_inotify_fd = -1;

	m_inotify_fd = inotify_init();
	result = inotify_add_watch(m_inotify_fd, DEVICE_PATH, IN_ALL_EVENTS/*IN_DELETE | IN_CREATE*/);
	if(result < 0) {
		printf("inotify_add_watch error: %s\n", strerror(errno));
		goto error_out;
	}

	for(;;) {
		get_notify(m_inotify_fd);
	}

error_out:
	close(m_inotify_fd);
	return 0;
}
