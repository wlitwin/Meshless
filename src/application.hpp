#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

// Defines the application's window title
extern const char* title;

/* Perform any initialization for this application. At this
 * point the OpenGL context is made and the window is open.
 */
bool initialize(int argc, char* argv[]);

/* Called when the user resizes the window. Also called right
 * after initialize.
 */
void resolution_changed(int width, int height);

/* Called to perform an application update. Usually every
 * 1/60 seconds.
 */
bool update(double dt);

/* Called after update, should put rendering code here.
 */
void render(void);

/* Called before the application exits.
 */
void cleanup(void);

#endif
