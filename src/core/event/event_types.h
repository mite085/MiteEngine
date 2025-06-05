#ifndef MITE_EVENT_TYPES
#define MITE_EVENT_TYPES

namespace mite {

// �¼����֧�������жϣ�
// �� flags = EventCategoryInput | EventCategoryKeyboard;
// ��ʾflags���������¼����Ǽ����¼���
// ʹ��flags & (EventCategoryInput | EventCategoryKeyboard) != 0,�ж�����Ƿ����
enum EventCategory {
  None = 0,
  EventCategoryWindow = 1 << 0,
  EventCategoryInput = 1 << 1,
  EventCategoryKeyboard = 1 << 2,
  EventCategoryMouse = 1 << 3,
  EventCategoryEditor = 1 << 4,  // �༭���ض��¼�����
  EventCategoryCustom = 1 << 5   // �û��Զ����¼�����
};

// �¼�����
enum class EventType {
  None = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,

  KeyPressed,
  KeyReleased,
  KeyTyped,

  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,

  AssetLoaded,      // �ʲ�����
  MaterialChanged,  // ���ʱ��
  EntitySelected,   // �༭���ض��¼�

  CustomEvent  // �û��Զ����¼�
};

}  // namespace mite

#endif
