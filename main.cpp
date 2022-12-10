#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

struct Camera {
	int y;  // 摄像机高度
	int curser_pos_x, curser_pos_y;
	int index;  //光标指向位置的index
	char* buffer;
	int array_len;
	int len;
};
struct Char {
	int index;  // 在buffer中的index
	unsigned char ch;
};
struct Line {
	int line_flag;  // 是不是回车敲出的行的结尾
	// char line[80];
	Char line[80];
	int len;
	int start_index;  // 行首索引
};
void goto_xy(int x, int y) {
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void insert_char(char* str, int pos, char ch, Camera* c) {
	if (strlen(str) + 1 > c->array_len) {
		str = (char*)realloc(c->buffer, c->array_len + 100);
		c->buffer = str;
		c->array_len += 100;
	}
	int i;
	for (i = strlen(str); i >= pos; i--) {
		str[i + 1] = str[i];
	}
	str[pos] = ch;
}
void insert_str(char* str, int pos, Camera* c) {
	for (int i = 0; i < strlen(str); i++) {
		insert_char(c->buffer, pos++, str[i], c);
	}
}
void delete_char(char* str, int pos) {
	int i;
	for (i = pos; i < strlen(str); i++) {
		str[i] = str[i + 1];
	}
}
class parse {
		Camera* camera;
		Line l[25];

	public:
		parse(Camera* c) {
			camera = c;
			clean();
		}
		void Set() {
			// 根据camera的y值
			clean();
			int l = 0;
			int sc = 0;
			int f = 0;
			int nl = 0;
			int len = 0;
			int sl = 0;
			int i;
			for (i = 0; i < strlen(camera->buffer) && nl < 25; i++) {
				// printf("%c", camera->buffer[i] == '\n' ? 'n' : camera->buffer[i]);
				if (l == camera->y) {
					// printf("OK1\n");
					if (sc == 0) {
						this->l[nl].start_index = i;
						if (nl != 0) {
						}
					}
					if (camera->buffer[i] == '\n' || sc == 80) {
						//  printf("\nN!!(%02x) sc=%d\n", camera->buffer[i], sc);
						this->l[nl].line_flag = 1;  //在这里设置
						this->l[nl].len = len;
						len = 0;
						sl = 0;
						nl++;
						f = sc == 80 ? 1 : 0;
						sc = 0;
					} else {
						//   printf("Y,sc=%d\n", sc);
						this->l[nl].line[sc++].ch = camera->buffer[i];
						this->l[nl].line[sc - 1].index = i;
						len++;
						if (sc == 80) {
							// what the fuck?
							//     printf("\nN!!(%02x) sc=%d\n", camera->buffer[i], sc);
							nl++;
							f = sc == 80 ? 1 : 0;
							sc = 0;
						}
					}

				} else {
					if (camera->buffer[i] == '\n' || sc == 80) {
						l++;
						f = sc == 80 ? 1 : 0;
						sc = 0;

					} else {
						sc++;
					}
				}
			}
			f = sc == 80 ? 1 : 0;
			sc = 0;
			if (sc == 0) {
				this->l[nl].line_flag = 1;  //在这里设置
				this->l[nl].len = len;
				this->l[nl].start_index = i;
				// this->l[nl - 1].start_index = sl;
				len = 0;
				sl = 0;
			}
		}
		Line* getBuf() {
			return l;
		}

	private:
		void clean() {
			for (int i = 0; i < 25; i++) {
				l[i].line_flag = 0;
				l[i].len = 0;
				l[i].start_index = 0;
				for (int j = 0; j < 80; j++) {
					l[i].line[j].index = 0;
					l[i].line[j].ch = 0;
				}
			}
		}
};
class render {
		char* buf;
		parse* p;
		Camera* camera;

	public:
		render(char* buffer, Camera* c, parse* _p) {
			buf = buffer;
			p = _p;
			camera = c;
		}
		void showAll() {
			Line* l = p->getBuf();
			goto_xy(0, 0);
			for (int i = 0; i < 25; i++) {
				for (int j = 0; j < 80; j++) {
					printf("%c", l[i].line[j].ch == '\0' ? ' ' : l[i].line[j].ch);
				}
				printf("\n");
			}
			// printf("X=%d,Y=%d\n", camera->curser_pos_x, camera->curser_pos_y);
			goto_xy(camera->curser_pos_x, camera->curser_pos_y);
		}
};
bool Need_Sroll(Line* l) {
	if (l[24].line_flag == 1 || l[24].line[0].ch != '\0') {
		return true;
	}
	return false;
}
int Show_Line_Max(Line* l) {
	int i;
	for (i = 0; i < 25; i++) {
		if (l[i].line[0].ch == '\0' && l[i].line_flag != 1) {
			return i;
		}
	}
	return i;
}
class Note {
		Camera* camera;
		parse* p;

	public:
		int maxLine() {
			int l = 0;
			int sc = 0;
			for (int i = 0; i < strlen(camera->buffer); i++) {
				if (camera->buffer[i] == '\n' || sc == 80) {
					l++;
					sc = 0;
				} else {
					sc++;
				}
			}
			return l;
		}

		Note(Camera* c, parse* _p) {
			camera = c;
			p = _p;
		}
		void Insert(char ch) {
			insert_char(camera->buffer, camera->index, ch, camera);
		}
		void Delete() {
			/* 判断3“0”情况 */
			if (camera->y == 0 && camera->curser_pos_x == 0 && camera->curser_pos_y == 0) {
				return;
			}
			delete_char(camera->buffer, camera->index);
		}
		/* 上下左右操作 */
		void up() {
			if (camera->y == 0 && camera->curser_pos_y == 0) {
				// 无法上移
				// printf("Can not up.\n");
				return;
			}
			if (camera->curser_pos_y == 0) {
				camera->y--;
			}
			p->Set();
			Line* l = p->getBuf();  // 获取当前行布局
			if (camera->curser_pos_y == 0) {
				if (l[0].len == 0) {
					camera->index = l[0].start_index;
				} else {
					camera->index = l[0].line[l[0].len - 1].index + 1;
				}
				camera->curser_pos_x = l[0].len;
				camera->curser_pos_y = 0;
			} else {
				camera->curser_pos_y--;
				if (l[camera->curser_pos_y].len == 0) {
					camera->index = l[camera->curser_pos_y].start_index;
				} else {
					camera->index = l[camera->curser_pos_y]
					                .line[l[camera->curser_pos_y].len - 1]
					                .index +
					                1;
				}
				camera->curser_pos_x = l[camera->curser_pos_y].len;
			}
		}
		int down() {
			Line* l;
			p->Set();
			l = p->getBuf();
			int ml = maxLine();
			if (camera->curser_pos_y != 24) {
				if (l[camera->curser_pos_y + 1].line[0].ch == '\0' &&
				        l[camera->curser_pos_y + 1].line_flag == 0) {
				//	  printf("Can not Down2.\n");
					return 0;
				}
			} else {
				if (ml < (camera->y + camera->curser_pos_y) + 1) {
					 // printf("Can not Down1. %d %d %d\n",ml,camera->y,camera->curser_pos_y);
				//	  for(;;);
					return 0;
				}
			}
			//  printf("sure!\n");
			if (camera->curser_pos_y == 24) {
				camera->y++;
			}
			 p->Set();
			l = p->getBuf();  // 获取当前行布局
			if (camera->curser_pos_y == 24) {
				if (l[24].len == 0) {
					camera->index = l[24].start_index;
				} else {
					camera->index = l[24].line[l[24].len - 1].index + 1;
				}
				camera->curser_pos_x = l[24].len;
				camera->curser_pos_y = 24;
			} else {
				camera->curser_pos_y++;
				if (l[camera->curser_pos_y].len == 0) {
					camera->index = l[camera->curser_pos_y].start_index;
				} else {
					camera->index = l[camera->curser_pos_y]
					                .line[l[camera->curser_pos_y].len - 1]
					                .index +
					                1;
					//     printf("INDEX=%d\n", camera->index);
				}
				camera->curser_pos_x = l[camera->curser_pos_y].len;
			}
			return 1;
		}
		void left() {
			if (camera->curser_pos_x == 0) {
				up();
				Line* l = p->getBuf();

			} else {
				camera->curser_pos_x--;
				camera->index--;
			}
		}
		void right(int b) {
			// printf("this.\n");
			p->Set();
			Line* l = p->getBuf();
			if (camera->curser_pos_x == 80 ||
			        (camera->curser_pos_x == l[camera->curser_pos_y].len && b)) {
				// printf("%d %d   s Y=%d\n", camera->curser_pos_x,
				//        l[camera->curser_pos_y].len, camera->curser_pos_y);
				// system("PAUSE");
				int ret = down();
				// system("PAUSE");

				if (ret) {
					if (b) {
						camera->curser_pos_x = 0;
						camera->index = l[camera->curser_pos_y].start_index - 1;
					} else {
						camera->curser_pos_x = 1;
						camera->index = l[camera->curser_pos_y].start_index + 1;
					}
				}
				// asm("nop");
			} else {
				camera->curser_pos_x++;
				camera->index++;
			}
		}
};
class Editor {
	public:
		char * Main() {
			system("cls");
			Camera* c = (Camera*)malloc(sizeof(Camera));
			c->buffer = (char*)malloc(1000);
			for (int i = 0; i < 1000; i++) {
				c->buffer[i] = 0;
			}
			c->array_len = 1000;
			c->len = 0;
			c->y = 0;
			c->curser_pos_x = 0;
			c->curser_pos_y = 0;
			c->index = 0;
			strcpy(c->buffer, "");
			parse* prse = new parse(c);
			prse->Set();
			Note* n = new Note(c, prse);
			Line* l = prse->getBuf();
			//  printf("%s\n", c->buffer);
			render* r = new render(c->buffer, c, prse);
			r->showAll();
			int times=0;
			for (;;) {
				int ch = _getch();
				if (ch == '\r') {
					n->Insert('\n');
					n->down();
					//l = prse->getBuf();
					if (c->curser_pos_x != 0) {
						c->curser_pos_x = 0;
						c->index = l[c->curser_pos_y].line[0].index; // Holy Fuck
					}

				} else if(ch == '\b') {
					n->left();
					n->Delete();
				} else if(ch == 0x1b) {
					return c->buffer;
				}
				else if (ch == 224) {
					ch = _getch();
					if (ch == 77) {
						n->right(1);
					} else if (ch == 75) {
						n->left();
					} else if (ch == 72) {
						n->up();
					} else if (ch == 80) {
						n->down();
					}
				} else {
					n->Insert(ch);
					n->right(0);
				}
				prse->Set();
				r->showAll();
			}
		}
};
int main(int argc, char** argv) {
	Editor* e = new Editor();
	char *c = e->Main();
	system("cls");
	char b[50];
	printf("File:");
	gets(b);
	FILE *fp = fopen(b,"wb");
	for(int i = 0;i<strlen(c);i++) {
		if(c[i] == '\n') {
			fputc('\r',fp);
			fputc('\n',fp);
		} else {
			fputc(c[i],fp);
		}
	}
	return 0;
}
