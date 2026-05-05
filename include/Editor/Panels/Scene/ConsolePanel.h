#ifndef CONSOLE_PANEL_H
#define CONSOLE_PANEL_H

#include "../../UI/Panel.h"
#include "RanakEngine/Log/LogManager.h"

#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <memory>

/**
 * @class ConsolePanel
 * @brief Editor panel that displays engine log messages with color-coded severity.
 *
 * Hooks into the engine Log::Manager via a listener callback so that all
 * logged messages (debug, normal, warning, error) appear in a scrollable
 * list.  Provides severity filters, a text search bar, and a clear button.
 */
class ConsolePanel : public Panel
{
    private:
    struct LogEntry
    {
        RanakEngine::Log::MessageContent::Severity severity;
        std::string message;
    };

    std::vector<LogEntry> m_entries; ///< All received log entries.
    std::mutex m_entriesMutex;       ///< Protects m_entries (listener fires from log thread).

    /// Shared liveness flag. Set to false in destructor so the log listener
    /// becomes a no-op after this panel is destroyed, preventing use-after-free.
    std::shared_ptr<std::atomic<bool>> m_alive;

    bool m_autoScroll = true;       ///< Scroll to bottom on new messages.
    bool m_showDebug   = true;
    bool m_showNormal  = true;
    bool m_showWarning = true;
    bool m_showError   = true;

    void Draw() override;
    
    public:
    /**
     * @brief Constructs the console and registers as a log listener.
     * @param _editor Reference to the owning Editor.
     */
    ConsolePanel(Editor& _editor);
    ~ConsolePanel();

    /** @brief Removes all stored messages. */
    void ClearLog();
};

#endif
