#include "MenuManager.h"
#include "Parameter.h"
MenuManager::MenuManager(DisplayInterface &display, Menu *menu, ButtonInterface *buttonInterface)
    : display(display),
      currentMenu(menu),
      buttonInterface(buttonInterface),
      currentIndex(0),
      currentScale(1)
{
}

void MenuManager::waitTillButtonReleased(Button bt)
{
    while (buttonInterface->getPressedButton() == bt)
        delay(10);
}

Button MenuManager::debounceButton()
{
    Button lastButton = buttonInterface->getPressedButton();
#if 1 // If encoder used - no need to additional debounce
    return lastButton;
#else
    delay(50); // Short delay for debounce
    if (lastButton == buttonInterface->getPressedButton())
        return lastButton;
    return BUTTON_NONE;
#endif
}

void MenuManager::clearAndSetupDisplay(int textSize, int cursorX, int cursorY)
{
    display.clear();
    display.setTextSize(textSize);
    display.setCursor(cursorX, cursorY);
}

void MenuManager::displayText(const char *text) { display.print(text); }

void MenuManager::displayParameterDetails(Parameter *parameter)
{
    clearAndSetupDisplay(2);
    displayText("SET x");
    display.print(currentScale);
    displayText("\n\n");

    displayText(parameter->getName());
    displayText("\n=");
    display.print(parameter->getValue()); // Display parameter value

    display.display();
}

void MenuManager::displayMenuItem(int index, const char *label, bool isSelected)
{
    display.setCursor(0, (index + 1) * 8); // Adjust y-position based on index
    if (isSelected)
        displayText("> ");
    else
        displayText("  ");
    displayText(label);
}

void MenuManager::switchScale()
{
    if (currentScale == 1)
        currentScale = 10;
    else if (currentScale == 10)
        currentScale = 100;
    else if (currentScale == 100)
        currentScale = 1000;
    else if (currentScale == 1000)
        currentScale = 10000;
    else
        currentScale = 1;
}

void MenuManager::handleInput(Button button)
{
    MenuItem *item = nullptr;

    waitTillButtonReleased(BUTTON_UP);

    switch (button)
    {
    case BUTTON_RIGHT:
        currentIndex--;
        if (currentIndex < 0)
        {
            currentIndex = currentMenu->getItemCount() - 1;
            topIndex = max(0, currentMenu->getItemCount() - maxVisibleItems);
        }

        if (currentIndex < topIndex)
            topIndex--;
        break;
    case BUTTON_LEFT:
        currentIndex++;
        if (currentIndex >= currentMenu->getItemCount())
        {
            currentIndex = 0;
            topIndex = 0;
        }

        if (currentIndex >= topIndex + maxVisibleItems)
            topIndex++;
        break;

    // case BUTTON_UP:
    case BUTTON_CENTER:
        item = currentMenu->getItem(currentIndex);
        if (item != nullptr)
        {
            if (item->getType() == MENU_ITEM_ACTION)
                item->executeAction();
            else if (item->getType() == MENU_ITEM_PARAMETER)
                displayParameter(item->getParameter());
        }
        waitTillButtonReleased(BUTTON_CENTER);
        break;

    case BUTTON_DOWN: // Implement "Back" functionality
    {
        // Forcly invoke last item (usualy Exit)
        waitTillButtonReleased(BUTTON_DOWN);
        int lastItem = currentMenu->getItemCount() - 1;
        item = currentMenu->getItem(lastItem);
        if (item != nullptr)
        {
            if (item->getType() == MENU_ITEM_ACTION)
                item->executeAction();
        }
    }
    break;

    default:
        break;
    }
    updateMenu();
}

void MenuManager::displayParameter(Parameter *parameter)
{
    bool tuneFlag = true;
    Button bt = BUTTON_NONE;
    waitTillButtonReleased(BUTTON_DOWN);

    while (tuneFlag)
    {
        bt = debounceButton();
        if (bt != BUTTON_NONE)
        {
            switch (bt)
            {
            case BUTTON_LEFT:
                parameter->decrement(currentScale);
                break;

            case BUTTON_RIGHT:
                parameter->increment(currentScale);
                break;

            case BUTTON_DOWN:
                tuneFlag = false; // Exit adjustment mode
                currentScale = 1;
                break;

            case BUTTON_CENTER: // Switch to the next scale
                switchScale();
                break;

            default:
                break;
            }
            displayParameterDetails(parameter); // Display the current scale
        }
    }
}

void MenuManager::updateMenu()
{
    clearAndSetupDisplay(1);
    displayText(currentMenu->getTitle());

    int itemCount = currentMenu->getItemCount();
    currentIndex = constrain(currentIndex, 0, itemCount - 1);
    topIndex = max(0, min(topIndex, itemCount - maxVisibleItems));

    int endIndex = min(topIndex + maxVisibleItems, itemCount);
    for (int i = topIndex; i < endIndex; i++)
    {
        bool isSelected = (i == currentIndex);
        displayMenuItem(i - topIndex, currentMenu->getItem(i)->getLabel(), isSelected);
    }
    display.display();
}
