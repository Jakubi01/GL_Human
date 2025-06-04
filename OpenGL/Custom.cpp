#include <GL/glut.h>
#include <cmath>
#include <cstdio>

constexpr int window_width = 800;
constexpr int window_height = 600;
constexpr int window_position_x = 300;
constexpr int window_position_y = 300;

struct GLfloat3
{
	GLfloat3(const GLfloat _x, const GLfloat _y, const GLfloat _z) 
		: x(_x)
		, y(_y)
		, z(_z)
		, target_pos_x(0.f)
		, target_pos_y(0.f)
		, target_pos_z(0.f)
	{}

	void SetTargetPos(const GLfloat3 TargetPos) const
	{
		target_pos_x = TargetPos.x;
		target_pos_y = TargetPos.y;
		target_pos_z = TargetPos.z;
	}

	void MoveToTargetPos(const GLfloat step) const
	{
		GLfloat dx = target_pos_x - x;
		GLfloat dy = target_pos_y - y;
		GLfloat dz = target_pos_z - z;

		if (fabs(dx) < 0.01f && fabs(dy) < 0.01f && fabs(dz) < 0.01f)
			return;

		x += dx * step;
		y += dy * step;
		z += dz * step;
	}

	GLfloat3 GetTargetPos() const { return GLfloat3(target_pos_x, target_pos_y, target_pos_z); }
	GLfloat3 GetPos() const {return GLfloat3(x, y, z);}

	bool IsNearlyEqual(const GLfloat3 Other) const
	{
		return x - Other.x < 0.1f && y - Other.y < 0.1f && z - Other.z < 0.1f;
	}

	bool operator == (const GLfloat3 Other) const
	{
		return IsNearlyEqual(Other);
	}
	
	mutable GLfloat x, y, z;
	mutable GLfloat target_pos_x, target_pos_y, target_pos_z;
};

// Timer를 위한 변수 선언
GLint angle_Left_upper = 0;
GLint angle_Left_lower = 0;
GLint angle_Right_upper = 0;
GLint angle_Right_lower = 0;

// 방향 각도
GLint dir_Left_upper = 1;
GLint dir_Left_lower = 1;
GLint dir_Right_upper = 1;
GLint dir_Right_lower = 1;


// 포즈 번호
static int pose_num = 0;
constexpr int pose_num_max = 4;

// 애니메이션
UINT8 bShouldPlayAnimation = false;
UINT8 bShouldMoveToTopView = false;
float AnimationSpeed = 0.05f;

GLfloat animation_angle = 0.f;
GLint animation_direction = 1;

// 포즈 별 카메라 위치
const GLfloat3 camera_position_FullBodyView(0.f, 0.f, 8.f);
const GLfloat3 camera_position_closeView(0.f, 0.f, 4.f);
const GLfloat3 camera_position_Left_SideView(5.f, 2.f, 4.f);
const GLfloat3 camera_position_Right_SideView(-4.f, 2.f, 4.f);
const GLfloat3 camera_position_TopView(0.f, 6.f, 0.f);
const GLfloat3 camera_position_Back_TopView(0.f, 5.f, -4.f);

// 현재 카메라 위치
GLfloat3 current_camera_position = camera_position_FullBodyView;

// 보는 방향, 시선이 보는 목적 지점
constexpr GLfloat camera_center_x = 0, camera_center_y = -1, camera_center_z = 0;

// 카메라 방향
constexpr GLfloat camera_up_x = 0, camera_up_y = 1, camera_up_z = 0;




void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	/*	뷰포트
		최종적으로 화면에 나타낼 화면 좌표계 변화
		위치 조정
		비율 조정
	*/
	constexpr GLfloat left = 0, bottom = 50;

	glViewport(static_cast<GLint>(left), static_cast<GLint>(bottom), width, height);

	// 원근 투영 함수 glMatrixMode();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// 비율 설정
	const GLfloat ratio = static_cast<float>(width) / static_cast<float>(height);

	// 원근 투영 거리를 2->10으로 설정
	gluPerspective(40, ratio, 0.1, 20);
}

void drawArrow()
{
	glBegin(GL_LINES);
	glVertex3f(0.f, 0.f, 0.f);  // 화살표 가운데 선
	glVertex3f(0.2f, 0.f, 0.f);
	glVertex3f(0.2f, 0.f, 0.f);  // 화살표 위쪽 대각선
	glVertex3f(0.14f, 0.06f, 0.f);
	glVertex3f(0.2f, 0.f, 0.f);  // 화살표 아래쪽 대각선
	glVertex3f(0.14f, -0.06f, 0.f);
	glEnd();
}

void drawAxis()
{
	glColor3f(1.f, 1.f, 1.f);

	drawArrow();
	
	glPushMatrix();
	glRotatef(90.f, 0.f, 0.f, 1.f);  // glRoatef(각, x축, y축, z축)
	drawArrow();
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90.f, 0.f, 1.f, 0.f);
	drawArrow();// 축 그리기 함수
	glPopMatrix();
}

void drawBody()
{
	// 좌표 측면 : 이동, 회전, 정육면체 생성
	// 모델 측면 : 정육면체 생성, 회전, 이동
	glPushMatrix();
	glScalef(2.f, 4.f, 1.f);
	glutWireCube(0.25); // 로봇 몸통
	glPopMatrix();
}

void drawCuboid(const GLfloat sx, const GLfloat sy, const GLfloat sz)
{
	glPushMatrix();
	glScalef(sx, sy, sz);
	glutWireCube(1);
	glPopMatrix();
}

void drawHead()
{
	glutWireSphere(0.2, 15, 15);
	glTranslatef(0.f, -0.7f, 0.f); // y 축 방향으로 이동
}

#pragma region LeftBody

// glRoatef()함수의 각도를 파라미터로 설정
void drawLeftUpperArm(const GLfloat angle)
{
	glTranslatef(0.35f, 0.5f, 0.f);
	glRotatef(-90.f, 0.f, 0.f, 1.f);
	glRotatef(angle, 0.f, 1.f, 0.f);
	glTranslatef(0.25f, 0.f, 0.f);
	drawCuboid(0.5f, 0.2f, 0.2f);
}

// glRoatef()함수의 각도를 파라미터로 설정
void drawLeftLowerArm(const GLfloat angle) 
{
	glTranslatef(0.25, 0, 0);
	glRotatef(-angle, 0.f, 1.f, 0.f);
	glTranslatef(0.25, 0, 0);
	drawCuboid(0.5f, 0.2f, 0.2f);
}

void drawLeftHand()
{
	glTranslatef(0.35f, 0.f, 0.f); // X 축 방향으로 이동
	glRotatef(90.f, 1.f, 0.f, 0.f);
	glutWireSphere(0.1, 15, 15);

	// finger 1
	glPushMatrix();
	glTranslatef(0.1f, 0.f, 0.f);  // x축 방향으로 이동
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 2
	glPushMatrix();
	glRotatef(30.f, 0.f, 0.f, 1.f);
	glTranslatef(0.1f, 0.f, 0.f);  // x축 방향으로 이동
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 3
	glPushMatrix();
	glRotatef(60.f, 0.f, 0.f, 1.f);
	glTranslatef(0.1f, 0.f, 0.f);  // x축 방향으로 이동
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 4
	glPushMatrix();
	glRotatef(-30.f, 0.f, 0.f, 1.f);
	glTranslatef(0.1f, 0.f, 0.f);  // x축 방향으로 이동
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 5
	glPushMatrix();
	glRotatef(-60.f, 0.f, 0.f, 1.f);
	glTranslatef(0.1f, 0.f, 0.f);  // x축 방향으로 이동
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();
}

void drawLeftUpperLeg(const GLfloat angle)
{
	glTranslatef(0.15f, -0.5f, 0.f);  // 골반에서 시작
	glRotatef(-angle, 1.f, 0.f, 0.f); // 앞뒤로 회전
	glTranslatef(0.f, -0.25f, 0.f);  // 허벅지 중심으로 이동
	drawCuboid(0.2f, 0.5f, 0.2f);    // 허벅지
}

void drawLeftLowerLeg(const GLfloat angle)
{
	glTranslatef(0.f, -0.25f, 0.f);  // 무릎 위치로 이동
	glRotatef(angle / 2, 1.f, 0.f, 0.f); // 종아리 회전
	glTranslatef(0.f, -0.25f, 0.f);
	drawCuboid(0.2f, 0.5f, 0.2f);    // 종아리
}

void drawLeftFoot()
{
	glTranslatef(0.f, -0.3f, 0.1f); // 발 위치
	drawCuboid(0.2f, 0.05f, 0.3f);   // 발
}

void drawLeftBody()
{
	// Draw left arm
	glPushMatrix();
	drawLeftUpperArm(static_cast<GLfloat>(angle_Left_upper));
	drawLeftLowerArm(static_cast<GLfloat>(angle_Left_lower));
	drawLeftHand();
	glPopMatrix();

	// Draw left leg
	glPushMatrix();
	drawLeftUpperLeg(static_cast<GLfloat>(angle_Left_upper));
	drawLeftLowerLeg(static_cast<GLfloat>(angle_Left_lower));
	drawLeftFoot();
	glPopMatrix();
}

#pragma endregion

#pragma region RightBody

void drawRightUpperArm(const GLfloat angle)
{
	glTranslatef(-0.35f, 0.5f, 0.f);
	glRotatef(90.f, 0.f, 0.f, 1.f);
	glRotatef(-angle, 0.f, -1.f, 0.f);
	glTranslatef(-0.25f, 0.f, 0.f);
	drawCuboid(0.5f, 0.2f, 0.2f);
}

void drawRightLowerArm(const GLfloat angle)
{
	glTranslatef(-0.25, 0, 0);
	glRotatef(-angle, 0.f, 1.f, 0.f);
	glTranslatef(-0.25, 0, 0);
	drawCuboid(0.5f, 0.2f, 0.2f);
}

void drawRightHand()
{
	glTranslatef(-0.35f, 0.f, 0.f); // 오른손 위치
	glRotatef(90.f, 1.f, 0.f, 0.f);
	glutWireSphere(0.1, 15, 15);

	// finger 1
	glPushMatrix();
	glTranslatef(-0.1f, 0.f, 0.f);
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 2
	glPushMatrix();
	glRotatef(-30.f, 0.f, 0.f, 1.f);
	glTranslatef(-0.1f, 0.f, 0.f);
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 3
	glPushMatrix();
	glRotatef(-60.f, 0.f, 0.f, 1.f);
	glTranslatef(-0.1f, 0.f, 0.f);
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 4
	glPushMatrix();
	glRotatef(30.f, 0.f, 0.f, 1.f);
	glTranslatef(-0.1f, 0.f, 0.f);
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();

	// finger 5
	glPushMatrix();
	glRotatef(60.f, 0.f, 0.f, 1.f);
	glTranslatef(-0.1f, 0.f, 0.f);
	drawCuboid(0.1f, 0.05f, 0.05f);
	glPopMatrix();
}

void drawRightUpperLeg(const GLfloat angle)
{
	glTranslatef(-0.13f, -0.5f, 0.f); // 오른쪽 골반
	glRotatef(angle, 1.f, 0.f, 0.f);
	glTranslatef(0.f, -0.25f, 0.f);
	drawCuboid(0.2f, 0.5f, 0.2f);
}

void drawRightLowerLeg(const GLfloat angle)
{
	glTranslatef(0.f, -0.25f, 0.f);
	glRotatef(-angle / 2, 1.f, 0.f, 0.f);
	glTranslatef(0.f, -0.25f, 0.f);
	drawCuboid(0.2f, 0.5f, 0.2f);
}

void drawRightFoot()
{
	glTranslatef(0.f, -0.3f, 0.1f);
	drawCuboid(0.2f, 0.05f, 0.3f);
}

void drawRightBody()
{
	glPushMatrix();
	drawRightUpperArm(static_cast<GLfloat>(angle_Right_upper));
	drawRightLowerArm(static_cast<GLfloat>(angle_Right_lower));
	drawRightHand();
	glPopMatrix();

	glPushMatrix();
	drawRightUpperLeg(static_cast<GLfloat>(angle_Right_upper));
	drawRightLowerLeg(static_cast<GLfloat>(angle_Right_lower));
	drawRightFoot();
	glPopMatrix();
}


#pragma endregion

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		current_camera_position.x, current_camera_position.y, current_camera_position.z,	// 시선의 위치
		camera_center_x, camera_center_y, camera_center_z,									// 보는 방향, 원점(0,0,0)
		camera_up_x, camera_up_y, camera_up_z												// 카메라 방향    (0,1,0)
		);

	drawHead();
	drawBody();

	drawLeftBody();
	drawRightBody();

	glutSwapBuffers();
}

void AnimationTimer(int value)
{
	if (!bShouldPlayAnimation)
	{
		return;
	}
	
	// 각도 변경
	animation_angle += static_cast<GLfloat>(animation_direction) * 2.f;

	// 각도 제한
	if (animation_angle > 30.f || animation_angle < -30.f)
	{
		animation_direction *= -1;
	}

	// 애니메이션 각도를 팔다리에 적용
	angle_Left_upper = static_cast<GLint>(animation_angle);
	angle_Left_lower = static_cast<GLint>(-animation_angle);
	angle_Right_upper = static_cast<GLint>(animation_angle);
	angle_Right_lower = static_cast<GLint>(-animation_angle);

	// 현재 창을 다시 표시
	glutPostRedisplay();
	
	// 카메라 변환 Timer
	glutTimerFunc(20, AnimationTimer, 1);
}

void SmoothCameraTransition(int value)
{
	if (bShouldMoveToTopView)
	{
		if (current_camera_position.GetTargetPos() == camera_position_TopView)
		{
			current_camera_position.SetTargetPos(camera_position_FullBodyView);
			glutTimerFunc(10, SmoothCameraTransition, 10);
		}

		if (current_camera_position.GetPos() == camera_position_FullBodyView)
		{
			bShouldMoveToTopView = false;
			current_camera_position.SetTargetPos(camera_position_TopView);
			glutTimerFunc(10, SmoothCameraTransition, 10);
		}
	}

	if (current_camera_position.GetTargetPos() == camera_position_Back_TopView)
	{
		AnimationSpeed = 0.01f;
	}
	
	current_camera_position.MoveToTargetPos(0.05f); // 부드럽게 이동

	glutPostRedisplay(); // 장면 다시 그리기

	// 목표 위치에 거의 도달했는지 확인
	GLfloat3 delta = current_camera_position.GetTargetPos();
	if (fabs(current_camera_position.x - delta.x) > 0.01f ||
		fabs(current_camera_position.y - delta.y) > 0.01f ||
		fabs(current_camera_position.z - delta.z) > 0.01f)
	{
		glutTimerFunc(16, SmoothCameraTransition, 1); // 60fps 정도
	}
	else
	{
		AnimationSpeed = 0.05f;
	}
}

void ChangePose(int value)
{
	switch (value)
	{
	case 0:
		current_camera_position.SetTargetPos(camera_position_Left_SideView);
		glutTimerFunc(16, SmoothCameraTransition, 1);
		break;
	case 1:
		current_camera_position.SetTargetPos(camera_position_Right_SideView);
		glutTimerFunc(16, SmoothCameraTransition, 1);
		break;
	case 2:
		current_camera_position.SetTargetPos(camera_position_TopView);
		bShouldMoveToTopView = true;
		glutTimerFunc(16, SmoothCameraTransition, 1);
		break;
	case 3:
		current_camera_position.SetTargetPos(camera_position_Back_TopView);
		glutTimerFunc(16, SmoothCameraTransition, 1);
		break;
	case 5:
		bShouldPlayAnimation = true;
		current_camera_position.SetTargetPos(camera_position_closeView);
		glutTimerFunc(16, SmoothCameraTransition, 1);
		break;
	default:
		break;
	}

	pose_num++;
	if (pose_num >= pose_num_max)
	{
		pose_num = 0;
	}
}

void MousePressed(int btn, int state, int x, int y)
{
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		ChangePose(pose_num);
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		bShouldPlayAnimation = !bShouldPlayAnimation;
		if (bShouldPlayAnimation)
		{
			glutTimerFunc(20, AnimationTimer, 1);
		}
	}
	else if (btn == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
	{
		ChangePose(pose_num = 5);
	}
}

void init()
{
	glClearColor(0.f, 0.f, 0.45f, 1.f); // Set background color
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(window_position_x, window_position_y);
	glutCreateWindow("Pose Simulation");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(MousePressed);

	glutMainLoop();
	return 0;
}