// /*
// 계층구조 모델링
// 1. 순방향 키네마틱스
// : 계층구조의 상위에서 점차 하위로 내려오면서 사용자가 직접 필요한 만큼의 각도를 지정하여 회전시킴으로써
// 최종적으로 원하는 자세를 만들어 내는 것
// -장점 : 사용자 마음대로 필요한 자세를 만들어 낼 수 있음
// -단점 : 각 관절마다 회전각을 사용자가 직접 입력해야 함
// 2. 역방향 키네마틱스
// : 계층구조의 가장 아래에 있는 물체 위치를 명시하면 상위 물체의 움직을 컴퓨터 내부에서 자동으로 계산되는 방법
// 예)  손이 그 위치에 존재하기 위해 나머지 모든 객체들이 해당 관절을 중심으로 몇 도를 회전해야 하는에 대한 계산
// */
//
// #include <GL/glut.h>
// #include <math.h>
//
// #define _WINDOW_WIDTH 300
// #define _WINDOW_HEIGHT 300
//
// // Timer를 위한 변수 선언
// int angle_upper = 0;
// int angle_lower = 0;
//
// // 방향 각도
// int dir_upper = 1;
// int dir_lower = 1;
//
// // 카메라 위치
// // GLfloat camx = 0.5, camy = 0.5, camz = 1;
// // GLfloat camx = 0.5, camy = 0.1, camz = 1;
// GLfloat camx = 0.5, camy = -0.5, camz = 4;
//
// // 보는 방향, 시선이 보는 목적 지점
// GLfloat cam2x = 0, cam2y = 0, cam2z = 0;
//
// // 카메라 방향
// GLfloat cam_upx = 0, cam_upy = 1, cam_upz = 0;
//
// void reshape(int width, int height) {
// 	glViewport(0, 0, width, height);
//
// 	/*
// 	GLfloat f_w = (GLfloat)width / _WINDOW_WIDTH;
// 	GLfloat f_h = (GLfloat)height / _WINDOW_HEIGHT;
// 	*/
//
// 	/* 뷰포트
// 	* 최종적으로 화면에 나타낼 화면 좌표계 변화
// 	* 위치 조정
// 	* 비율 조정
// 	*/
// 	GLfloat left = 0, bottom = 50;
//
// 	glViewport(left, bottom, width, height);
//
// 	glMatrixMode(GL_PROJECTION);  // 원근 투영 관련 함수 glMatrixMode();
// 	glLoadIdentity();
//
// 	/*
// 	GL_MODELVIEW : 특정 좌표 (0,0,0)에 도형을 그린다면 GL_MODELVIEW
// 					매트릭스를 곱해서 실제적인 위치 지정
// 					GL_MODELVIEW 에서 그려진 도형에 대한 실제 위치
// 	GL_PROJECTION :  매트릭트를 곱해서 최종적으로 어떻게 화면에 뿌릴 것인가를
// 					계산하는 부분임.
// 	*/
// 	/*
// 	직교 투영 : (orthographic transformation) 평면 투영으로 원근감이
// 				전혀 없는 상태
// 				glOrthof(x최소, x최대, y최소, y최대, near, far) 등을 통해서
// 				설정
// 	원근 투영 : gluPerspective(시야각, 종회비(뷰포트 또는 창의 너비/높이)
// 								,  앞면의 z, 뒤면의 z);
// 	*/
//
// 	// 직교 투영
// 	// glOrtho(-1.1 * f_w, 1.1 * f_w, -1.1 * f_h, 1.1 * f_h, -2.0, 2.0);
//
// 	// 비율 설정
// 	GLfloat ratio = (float)width / height;
//
// 	// 원근 투영
// 	gluPerspective(40, ratio, 0.1, 10); // 거리를 2->10으로 설정
//
// 	// gluLookAt(0.5, 0.5, 1, 0, 0, 0, 0, 1, 0);  // 기본 화면
// 	/*
// 	gluLookAt(camx, camy, camz,		      // 시선의 위치
// 		cam2x, cam2y, cam2z,		  // 보는 방향, 원점(0,0,0)
// 		cam_upx, cam_upy, cam_upz); // 카메라 방향
// 		*/
// 		/*
// 		reshape()함수내 gluLookAt() 함수가 있다는 것은 오브젝트가 움직일 때마다
// 		카메라를 조정해야함
// 		*/
// }
//
// void drawArrow() {
// 	glBegin(GL_LINES);
// 	glVertex3f(0, 0, 0);  // 화살표 가운데 선
// 	glVertex3f(0.2, 0, 0);
// 	glVertex3f(0.2, 0, 0);  // 화살표 위쪽 대각선
// 	glVertex3f(0.14, 0.06, 0);
// 	glVertex3f(0.2, 0, 0);  // 화살표 아래쪽 대각선
// 	glVertex3f(0.14, -0.06, 0);
// 	glEnd();
// }
//
//
// void drawAxis() {
// 	glColor3f(1, 1, 1);
//
// 	// 메트릭스 초기화
//
// 	//glMatrixMode(GL_MODELVIEW);
// 	//glLoadIdentity();
//
// 	drawArrow();
//
//
// 	glPushMatrix();
// 	glRotatef(90, 0, 0, 1);  // glRoatef(각, x축, y축, z축)
// 	drawArrow();
// 	glPopMatrix();
//
// 	glPushMatrix();
// 	glRotatef(-90, 0, 1, 0);
// 	drawArrow();// 축 그리기 함수
// 	glPopMatrix();
//
//
// }
//
// void drawBody() {
// 	// 좌표 측면 : 이동, 회전, 정육면체 생성
// 	// 모델 측면 : 정육면체 생성, 회전, 이동
// 	glPushMatrix();
// 	glScalef(2, 4, 1);
// 	glutWireCube(0.25); // 로봇 몸통
// 	glPopMatrix();
// }
//
// void drawCuboid(GLfloat sx, GLfloat sy, GLfloat sz) {
// 	glPushMatrix();
// 	glScalef(sx, sy, sz);
// 	glutWireCube(1);
// 	glPopMatrix();
// }
//
//
// void drawHead() {
// 	glutWireSphere(0.2, 15, 15);
// 	glTranslatef(0, -0.7, 0); // X 축 방향으로 이동
// }
//
// void drawUpperArm(GLfloat angle) {  // glRoatef()함수의 각도를 파라미터로 설정
//
// 	glTranslatef(0.25, 0.3, 0);  // 팔을 어깨쪽으로 이동
// 	glRotatef(angle, 0, 0, 1);
// 	glTranslatef(0.25, 0, 0);
//
// 	drawCuboid(0.5, 0.2, 0.2);
// }
//
// void drawLowerArm(GLfloat angle) { // glRoatef()함수의 각도를 파라미터로 설정
//
//
// 	//drawAxis(); // drawUpperArm()  함수와 도일한 위치에 존재하는 화살표
//
// 	glTranslatef(0.25, 0, 0);  // x축 방향으로 이동
//
// 	glRotatef(angle, 0, 0, 1);  // Z축 방향으로 30도 변경
//
// 	glTranslatef(0.25, 0, 0);  // x축 방향으로 이동
//
// 	//drawAxis();
//
// 	drawCuboid(0.5, 0.2, 0.2);
//
// }
//
// void drawLeftHand() {
// 	glTranslatef(0.35, 0, 0); // X 축 방향으로 이동
// 	glutWireSphere(0.1, 15, 15);
//
// 	drawAxis();
// }
//
// void drawFinger1() {
//
// 	glTranslatef(0.1, 0, 0);  // x축 방향으로 이동
//
// 	drawCuboid(0.1, 0.05, 0.05);
// }
//
// void drawFinger2() {
//
// 	glRotatef(30, 0, 0, 1);
//
// 	glTranslatef(0.1, 0, 0);  // x축 방향으로 이동
//
// 	drawCuboid(0.1, 0.05, 0.05);
// }
//
// void drawFinger3() {
//
// 	glRotatef(60, 0, 0, 1);
//
// 	glTranslatef(0.1, 0, 0);  // x축 방향으로 이동
//
// 	drawCuboid(0.1, 0.05, 0.05);
// }
//
// void drawFinger4() {
//
// 	glRotatef(-30, 0, 0, 1);
//
// 	glTranslatef(0.1, 0, 0);  // x축 방향으로 이동
//
// 	drawCuboid(0.1, 0.05, 0.05);
// }
//
// void drawFinger5() {
//
// 	glRotatef(-60, 0, 0, 1);
//
// 	glTranslatef(0.1, 0, 0);  // x축 방향으로 이동
//
// 	drawCuboid(0.1, 0.05, 0.05);
// }
//
//
//
//
// void MyDisplay() {
// 	glClear(GL_COLOR_BUFFER_BIT);
//
// 	glMatrixMode(GL_MODELVIEW);
// 	glLoadIdentity();
//
// 	gluLookAt(camx, camy, camz,		 // 시선의 위치
// 		cam2x, cam2y, cam2z,		 // 보는 방향, 원점(0,0,0)
// 		cam_upx, cam_upy, cam_upz); // 카메라 방향
//
// 	drawHead();
//
// 	drawBody();
//
// 	drawUpperArm(angle_upper);
// 	drawLowerArm(angle_lower);
//
// 	drawLeftHand();
// 	glPushMatrix();
// 	drawFinger1();
// 	glPopMatrix();
//
// 	glPushMatrix();
// 	drawFinger2();
// 	glPopMatrix();
//
// 	glPushMatrix();
// 	drawFinger3();
// 	glPopMatrix();
//
// 	glPushMatrix();
// 	drawFinger4();
// 	glPopMatrix();
//
// 	glPushMatrix();
// 	drawFinger5();
// 	glPopMatrix();
//
// 	// glutinitDisplayModel(GLUT_DOUBLE)로 할 경우 glFlush(); 대신
// 	//  glutSwapBuffers(); 로 변환하여야함 -> 애니메이션 기초 작업
// 	// glFlush();
// 	glutSwapBuffers();
// }
//
// void MyTimer(int value) {
// 	// 위쪽 팔과 아래쪽 팔을 움직이기 위한 변수 선언 필요
// 	angle_upper += dir_upper;
// 	angle_lower += 5 * dir_lower;
//
// 	if (angle_upper >= 80)
// 		dir_upper = -1;
// 	else if (angle_upper < -90)
// 		dir_upper = 1;
//
// 	if (angle_lower >= 160)
// 		dir_upper = -1;
// 	else if (angle_lower < 0)
// 		dir_upper = 1;
//
//
// 	GLfloat theta = 0.001;
// 	camx = camx * cos(theta) + camz * sin(theta);
// 	camz = camx * sin(theta) + camz * cos(theta);
//
// 	// 카메라 변환에 새로운 Timer 추가
// 	glutTimerFunc(20, MyTimer, 1);
//
// 	// 현재 창을 다시 표시해야 하는 것으로 표시 위해 코드 추가
// 	glutPostRedisplay();
// }
//
// int main(int argc, char** argv) {
// 	glutInit(&argc, argv);
// 	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
// 	glutInitWindowPosition(_WINDOW_WIDTH, _WINDOW_HEIGHT);
// 	glutCreateWindow("Arrow");
//
// 	glutDisplayFunc(MyDisplay);
// 	glutReshapeFunc(reshape);
//
//
// 	// Timer() 설정
// 	glutTimerFunc(20, MyTimer, 1);
//
// 	glutMainLoop();
// 	return 0;
//
// }