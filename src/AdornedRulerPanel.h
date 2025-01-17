/**********************************************************************

  Audacity: A Digital Audio Editor

  AdornedRulerPanel.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_ADORNED_RULER_PANEL__
#define __AUDACITY_ADORNED_RULER_PANEL__

#include "CellularPanel.h"
#include "widgets/Ruler.h" // member variable
#include "Prefs.h"
#include "ViewInfo.h" // for PlayRegion

class AudacityProject;
struct SelectedRegionEvent;
class TrackList;

// This is an Audacity Specific ruler panel.
class AUDACITY_DLL_API AdornedRulerPanel final
: public CellularPanel
, private PrefsListener
{
public:
   static AdornedRulerPanel &Get( AudacityProject &project );
   static const AdornedRulerPanel &Get( const AudacityProject &project );
   static void Destroy( AudacityProject &project );

   AdornedRulerPanel(AudacityProject *project,
                     wxWindow* parent,
                     wxWindowID id,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     ViewInfo *viewinfo = NULL);

   ~AdornedRulerPanel();

   void Refresh
      (bool eraseBackground = true, const wxRect *rect = (const wxRect *) NULL)
      override;

   bool AcceptsFocus() const override { return s_AcceptsFocus; }
   bool AcceptsFocusFromKeyboard() const override { return true; }
   void SetFocusFromKbd() override;

public:
   int GetRulerHeight() { return GetRulerHeight( ShowingScrubRuler() ); }
   static int GetRulerHeight(bool showScrubBar);
   wxRect GetInnerRect() const { return mInner; }

   void SetLeftOffset(int offset);

   void DrawSelection();

   void SetPlayRegion(double playRegionStart, double playRegionEnd);
   void ClearPlayRegion();
   void TogglePinnedHead();

   void GetMaxSize(wxCoord *width, wxCoord *height);

   void InvalidateRuler();

   void UpdatePrefs() override;
   void ReCreateButtons();

   void UpdateQuickPlayPos(wxCoord &mousePosX, bool shiftDown);

   bool ShowingScrubRuler() const;
   //void OnToggleScrubRulerFromMenu(wxCommandEvent& );
   bool SetPanelSize();
   
   void DrawBothOverlays();


private:
   void DoIdle();
   void OnIdle( wxIdleEvent &evt );
   void OnAudioStartStop(wxCommandEvent & evt);
   void OnPaint(wxPaintEvent &evt);
   void OnSize(wxSizeEvent &evt);
   void OnThemeChange(wxCommandEvent& evt);
   void OnSelectionChange(SelectedRegionEvent& evt);
   void DoSelectionChange( const SelectedRegion &selectedRegion );
   bool UpdateRects();
   void HandleQPClick(wxMouseEvent &event, wxCoord mousePosX);
   void HandleQPDrag(wxMouseEvent &event, wxCoord mousePosX);
   void HandleQPRelease(wxMouseEvent &event);
   void StartQPPlay(bool looped, bool cutPreview);

   void DoDrawBackground(wxDC * dc);
   void DoDrawEdge(wxDC *dc);
   void DoDrawMarks(wxDC * dc, bool /*text */ );
   void DoDrawPlayRegion(wxDC * dc);

public:
   void DoDrawScrubIndicator(wxDC * dc, wxCoord xx, int width, bool scrub, bool seek);
   void UpdateButtonStates();

private:
   static bool s_AcceptsFocus;
   struct Resetter { void operator () (bool *p) const { if(p) *p = false; } };
   using TempAllowFocus = std::unique_ptr<bool, Resetter>;

public:
   static TempAllowFocus TemporarilyAllowFocus();

private:
   enum class MenuChoice { QuickPlay, Scrub };
   void ShowContextMenu( MenuChoice choice, const wxPoint *pPosition);

   double Pos2Time(int p, bool ignoreFisheye = false);
   int Time2Pos(double t, bool ignoreFisheye = false);

   bool IsWithinMarker(int mousePosX, double markerTime);

private:

   Ruler mRuler;
   AudacityProject *const mProject;
   TrackList *mTracks;

   wxRect mOuter;
   wxRect mScrubZone;
   wxRect mInner;

   int mLeftOffset;  // Number of pixels before we hit the 'zero position'.


   double mIndTime;
   double mQuickPlayPosUnsnapped;
   double mQuickPlayPos;

   bool mIsSnapped;

   PlayRegion mOldPlayRegion;

   bool mIsRecording;

   //
   // Pop-up menu
   //
   void ShowMenu(const wxPoint & pos);
   void ShowScrubMenu(const wxPoint & pos);
   void DragSelection();
   void HandleSnapping();
   void OnSyncSelToQuickPlay(wxCommandEvent &evt);
   //void OnTimelineToolTips(wxCommandEvent &evt);
   void OnAutoScroll(wxCommandEvent &evt);
   void OnTogglePlayRegion(wxCommandEvent &evt);
   void OnClearPlayRegion(wxCommandEvent &evt);
   void OnSetPlayRegionToSelection(wxCommandEvent &evt);

   void OnPinnedButton(wxCommandEvent & event);
   void OnTogglePinnedState(wxCommandEvent & event);

   bool mPlayRegionDragsSelection;
   bool mTimelineToolTip;

   enum MouseEventState {
      mesNone,
      mesDraggingPlayRegionStart,
      mesDraggingPlayRegionEnd,
      mesSelectingPlayRegionClick,
      mesSelectingPlayRegionRange
   };

   MouseEventState mMouseEventState;
   double mLeftDownClickUnsnapped;  // click position in seconds, before snap
   double mLeftDownClick;  // click position in seconds
   bool mIsDragging;

   DECLARE_EVENT_TABLE()

   wxWindow *mButtons[3];
   bool mNeedButtonUpdate { true };

   //
   // CellularPanel implementation
   //

   // Get the root object defining a recursive subdivision of the panel's
   // area into cells
   std::shared_ptr<TrackPanelNode> Root() override;
public:
   AudacityProject * GetProject() const override;
private:
   TrackPanelCell *GetFocusedCell() override;
   void SetFocusedCell() override;
   void ProcessUIHandleResult
      (TrackPanelCell *pClickedTrack, TrackPanelCell *pLatestCell,
       unsigned refreshResult) override;

   void UpdateStatusMessage( const TranslatableString & ) override;

   void CreateOverlays();

   // Cooperating objects
   class TrackPanelGuidelineOverlay;
   std::shared_ptr<TrackPanelGuidelineOverlay> mOverlay;

   class ScrubbingRulerOverlay;
   
private:
   class CommonRulerHandle;
   class QPHandle;
   class PlayRegionAdjustingHandle;
   class ScrubbingHandle;

   class CommonCell;

   class QPCell;
   std::shared_ptr<QPCell> mQPCell;
   
   class ScrubbingCell;
   std::shared_ptr<ScrubbingCell> mScrubbingCell;

   // classes implementing subdivision for CellularPanel
   struct Subgroup;
   struct MainGroup;

   std::pair<double, double> mLastDrawnPlayRegion{};
   bool mLastPlayRegionActive = false;
   double mLastDrawnH{};
   double mLastDrawnZoom{};
   bool mDirtyPlayRegion{};
};

#endif //define __AUDACITY_ADORNED_RULER_PANEL__
