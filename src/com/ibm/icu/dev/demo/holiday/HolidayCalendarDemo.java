/*
 *******************************************************************************
 * Copyright (C) 1996-2000, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 *
 * $Source: /xsrl/Nsvn/icu/icu4j/src/com/ibm/icu/dev/demo/holiday/HolidayCalendarDemo.java,v $ 
 * $Date: 2002/02/19 04:10:23 $ 
 * $Revision: 1.9 $
 *
 *****************************************************************************************
 */

package com.ibm.icu.dev.demo.holiday;

import com.ibm.icu.dev.demo.*;
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
//import java.util.*;
import java.net.*;
import java.io.*;

//import java.text.SimpleDateFormat;
import com.ibm.icu.text.SimpleDateFormat;
import java.text.DateFormatSymbols;
//import java.util.SimpleTimeZone;
import com.ibm.icu.util.SimpleTimeZone;
import java.util.Locale;
import java.util.Vector;
import java.util.Date;

import com.ibm.icu.util.*;
import com.ibm.icu.dev.demo.impl.*;

/**
 * CalendarDemo demonstrates how Calendar works.
 */
public class HolidayCalendarDemo extends DemoApplet 
{
    /**
     * The main function which defines the behavior of the CalendarDemo
     * applet when an applet is started.
     */
    public static void main(String argv[]) {

        new HolidayCalendarDemo().showDemo();
    }

    /* This creates a CalendarFrame for the demo applet. */
    public Frame createDemoFrame(DemoApplet applet) {
        return new CalendarFrame(applet);
    }

	/**
	* A Frame is a top-level window with a title. The default layout for a frame
	* is BorderLayout.  The CalendarFrame class defines the window layout of
	* CalendarDemo.
	*/
	private static class CalendarFrame extends Frame implements ActionListener,
                                                                ItemListener
	{
    	private static final String creditString = "";

    	private static final boolean DEBUG = false;

    	private Locale curLocale = Locale.US;

    	private DemoApplet applet;

    	private static final Locale[] calendars = {
        	//new Locale("de","AT"),
        	Locale.CANADA,
        	Locale.CANADA_FRENCH,
        	Locale.FRANCE,
        	Locale.GERMANY,
        	new Locale("iw","IL"),
        	new Locale("el","GR"),
        	//new Locale("es","MX"),
        	Locale.UK,
        	Locale.US,
    	};
    	private static final Locale[] displays = {
        	Locale.CANADA,
        	Locale.UK,
        	Locale.US,
        	Locale.FRANCE,
        	Locale.CANADA_FRENCH,
        	//new Locale("de","AT"),
        	Locale.GERMAN,
        	new Locale("el","GR"),
        	//new Locale("iw","IL"),
        	new Locale("es","MX"),
    	};

    	/**
    	* Constructs a new CalendarFrame that is initially invisible.
    	*/
    	public CalendarFrame(DemoApplet applet)
    	{
        	super("Calendar Demo");
        	this.applet = applet;
        	init();
        	start();
        	enableEvents(WindowEvent.WINDOW_CLOSING);
    	}

    	/**
    	* Initializes the applet. You never need to call this directly, it
    	* is called automatically by the system once the applet is created.
    	*/
    	public void init()
    	{
        	// Get G7 locales only for demo purpose. To get all the locales
        	// supported, switch to calling Calendar.getAvailableLocales().
        	// commented
        	locales = displays;

        	buildGUI();
    	}

    	//------------------------------------------------------------
    	// package private
    	//------------------------------------------------------------
    	void addWithFont(Container container, Component foo, Font font) {
        	if (font != null)
            	foo.setFont(font);
        	container.add(foo);
    	}

    	/**
    	* Called to start the applet. You never need to call this method
    	* directly, it is called when the applet's document is visited.
    	*/
    	public void start()
    	{
        	// do nothing
    	}

    	private Choice          localeMenu;
    	private Choice          displayMenu;
    	private Locale[]        locales;

    	private Label           monthLabel;
    	private Button          prevYear;
    	private Button          prevMonth;
    	private Button          gotoToday;
    	private Button          nextMonth;
    	private Button          nextYear;
    	private CalendarPanel   calendarPanel;

    	private static final Locale kFirstLocale = Locale.US;

    	private static void add(Container container, Component component,
                            	GridBagLayout g, GridBagConstraints c)
    	{
        	g.setConstraints(component, c);
        	container.add(component);
    	}

    	public void buildGUI()
    	{
        	setBackground(DemoUtility.bgColor);
        	setLayout(new BorderLayout(10,10));

        	// Label for the demo's title
        	Label titleLabel = new Label("Calendar Demo", Label.CENTER);
        	titleLabel.setFont(DemoUtility.titleFont);

        	// Label for the current month name
        	monthLabel = new Label("", Label.LEFT);
        	monthLabel.setFont(new Font(DemoUtility.titleFont.getName(),
                                    	DemoUtility.titleFont.getStyle(),
                                    	(DemoUtility.titleFont.getSize() * 3)/2));

        	// Make the locale popup menus
        	localeMenu= new Choice();
        	localeMenu.addItemListener(this);
        	int selectMe = 0;
        	
        	for (int i = 0; i < calendars.length; i++) {
            	if (i > 0 &&
                    	calendars[i].getCountry().equals(calendars[i-1].getCountry()) ||
                	i < calendars.length - 1 &&
                    	calendars[i].getCountry().equals(calendars[i+1].getCountry()))
            	{
                	localeMenu.addItem(calendars[i].getDisplayCountry() + " (" +
                                	calendars[i].getDisplayLanguage() + ")");
            	} else {
                	localeMenu.addItem( calendars[i].getDisplayCountry() );
            	}
            	
            	if (calendars[i].equals(kFirstLocale)) {
                	selectMe = i;
            	}
        	}
        	
        	localeMenu.setBackground(DemoUtility.choiceColor);
        	localeMenu.select(selectMe);

        	displayMenu = new Choice();
        	displayMenu.addItemListener(this);
        	
        	selectMe = 0;
        	for (int i = 0; i < locales.length; i++) {
            	if (i > 0 &&
                    	locales[i].getLanguage().equals(locales[i-1].getLanguage()) ||
                	i < locales.length - 1 &&
                    	locales[i].getLanguage().equals(locales[i+1].getLanguage()))
            	{
                	displayMenu.addItem( locales[i].getDisplayName() );
            	} else {
                	displayMenu.addItem( locales[i].getDisplayLanguage());
            	}
            	
            	if (locales[i].equals(kFirstLocale)) {
            	    selectMe = i;
            	}
        	}
        	
        	displayMenu.setBackground(DemoUtility.choiceColor);
        	displayMenu.select(selectMe);

        	// Make all the next/previous/today buttons
        	prevYear = new Button("<<");
        	prevYear.addActionListener(this);
        	prevMonth = new Button("<");
        	prevMonth.addActionListener(this);
        	gotoToday = new Button("Today");
        	gotoToday.addActionListener(this);
        	nextMonth = new Button(">");
        	nextMonth.addActionListener(this);
        	nextYear = new Button(">>");
        	nextYear.addActionListener(this);

        	// The month name and the control buttons are bunched together
        	Panel monthPanel = new Panel();
        	{
            	GridBagLayout g = new GridBagLayout();
            	GridBagConstraints c = new GridBagConstraints();
            	monthPanel.setLayout(g);

            	c.weightx = 1;
            	c.weighty = 1;

            	c.gridwidth = 1;
            	c.fill = GridBagConstraints.HORIZONTAL;
            	c.gridwidth = GridBagConstraints.REMAINDER;
            	add(monthPanel, monthLabel, g, c);

            	c.gridwidth = 1;
            	add(monthPanel, prevYear, g, c);
            	add(monthPanel, prevMonth, g, c);
            	add(monthPanel, gotoToday, g, c);
            	add(monthPanel, nextMonth, g, c);
            	c.gridwidth = GridBagConstraints.REMAINDER;
            	add(monthPanel, nextYear, g, c);
        	}

        	// Stick the menu and buttons in a little "control panel"
        	Panel menuPanel = new Panel();
        	{
            	GridBagLayout g = new GridBagLayout();
            	GridBagConstraints c = new GridBagConstraints();
            	menuPanel.setLayout(g);

            	c.weightx = 1;
            	c.weighty = 1;

            	c.fill = GridBagConstraints.HORIZONTAL;

            	c.gridwidth = GridBagConstraints.RELATIVE;
            	Label l1 = new Label("Holidays");
            	l1.setFont(DemoUtility.labelFont);
            	add(menuPanel, l1, g, c);

            	c.gridwidth = GridBagConstraints.REMAINDER;
            	add(menuPanel, localeMenu, g, c);

            	c.gridwidth = GridBagConstraints.RELATIVE;
            	Label l2 = new Label("Display:");
            	l2.setFont(DemoUtility.labelFont);
            	add(menuPanel, l2, g, c);

            	c.gridwidth = GridBagConstraints.REMAINDER;
            	add(menuPanel, displayMenu, g, c);
        	}

        	// The title, buttons, etc. go in a panel at the top of the window
        	Panel topPanel = new Panel();
        	{
            	topPanel.setLayout(new BorderLayout());

            	//topPanel.add("North", titleLabel);
            	topPanel.add("Center", monthPanel);
            	topPanel.add("East", menuPanel);
        	}
        	add("North", topPanel);

        	// The copyright notice goes at the bottom of the window
        	Label copyright = new Label(DemoUtility.copyright1, Label.LEFT);
        	copyright.setFont(DemoUtility.creditFont);
        	add("South", copyright);

        	// Now create the big calendar panel and stick it in the middle
        	calendarPanel = new CalendarPanel( kFirstLocale );
        	add("Center", calendarPanel);

        	updateMonthName();
    	}

    	private void updateMonthName()
    	{
        	SimpleDateFormat f = new SimpleDateFormat("MMMM yyyyy",
                                                    	calendarPanel.getDisplayLocale());
        	f.setCalendar(calendarPanel.getCalendar());
        	f.setTimeZone(new SimpleTimeZone(0, "UTC"));        // JDK 1.1.2 workaround
        	monthLabel.setText( f.format( calendarPanel.firstOfMonth() ));
    	}
    	
    	/**
    	* Handles the event. Returns true if the event is handled and should not
    	* be passed to the parent of this component. The default event handler
    	* calls some helper methods to make life easier on the programmer.
    	*/
    	public void actionPerformed(ActionEvent e)
    	{
    	    Object obj = e.getSource();
    	    
    	    // *** Button events are handled here.
        	if (obj instanceof Button) {
            	if (obj == nextMonth) {
                	calendarPanel.add(Calendar.MONTH, +1);
            	}
            	else
            	if (obj == prevMonth) {
                	calendarPanel.add(Calendar.MONTH, -1);
            	}
            	else
            	if (obj == prevYear) {
                	calendarPanel.add(Calendar.YEAR, -1);
            	}
            	else
            	if (obj == nextYear) {
                	calendarPanel.add(Calendar.YEAR, +1);
            	}
            	else
            	if (obj == gotoToday) {
                	calendarPanel.set( new Date() );
            	}
            	updateMonthName();
        	}
    	}
    	
    	public void itemStateChanged(ItemEvent e)
        {
            Object obj = e.getSource();
            if (obj == localeMenu) {
            	calendarPanel.setCalendarLocale(calendars[localeMenu.getSelectedIndex()]);
            	updateMonthName();
        	}
        	else 
        	    if (obj == displayMenu) {
            	    calendarPanel.setDisplayLocale(locales[displayMenu.getSelectedIndex()]);
            	    updateMonthName();
            	}
        }
        
        /**
    	* Print out the error message while debugging this program.
    	*/
    	public void errorText(String s)
    	{
        	if (DEBUG)
        	{
            	System.out.println(s);
        	}
    	}
    	
    	protected void processWindowEvent(WindowEvent e)
        {
            System.out.println("event " + e);
            if (e.getID() == WindowEvent.WINDOW_CLOSING) {
            	this.hide();
            	this.dispose();

            	if (applet != null) {
            	    applet.demoClosed();
            	} else {
                    System.exit(0);
            	}
        	}
        }
	}


	private static class CalendarPanel extends Canvas {

    	public CalendarPanel( Locale locale ) {
        	set(locale, locale, new Date());
    	}

    	public void setCalendarLocale(Locale locale) {
        	set(locale, fDisplayLocale, fCalendar.getTime());
    	}

    	public void setDisplayLocale(Locale locale) {
        	set(fCalendarLocale, locale, fCalendar.getTime());
    	}

    	public void set(Date date) {
        	set(fCalendarLocale, fDisplayLocale, date);
    	}

    	public void set(Locale loc, Locale display, Date date)
    	{
        	if (fCalendarLocale == null || !loc.equals(fCalendarLocale)) {
            	fCalendarLocale = loc;
            	fCalendar = Calendar.getInstance(fCalendarLocale);
            	fAllHolidays = Holiday.getHolidays(fCalendarLocale);
        	}
        	if (fDisplayLocale == null || !display.equals(fDisplayLocale)) {
            	fDisplayLocale = display;
            	fSymbols = new DateFormatSymbols(fDisplayLocale);
        	}

        	fStartOfMonth = date;

        	dirty = true;
        	repaint();
    	}

    	public void add(int field, int delta)
    	{
        	synchronized(fCalendar) {
            	fCalendar.setTime(fStartOfMonth);
            	fCalendar.add(field, delta);
            	fStartOfMonth = fCalendar.getTime();
        	}
        	dirty = true;
        	repaint();
    	}

    	public com.ibm.icu.util.Calendar getCalendar() {
        	return fCalendar;
    	}

    	public Locale getCalendarLocale() {
        	return fCalendarLocale;
    	}

    	public Locale getDisplayLocale() {
        	return fDisplayLocale;
    	}


    	public Date firstOfMonth() {
        	return fStartOfMonth;
    	}

    	private Date startOfMonth(Date dateInMonth)
    	{
        	synchronized(fCalendar) {
            	fCalendar.setTime(dateInMonth);             // TODO: synchronization

            	int era = fCalendar.get(Calendar.ERA);
            	int year = fCalendar.get(Calendar.YEAR);
            	int month = fCalendar.get(Calendar.MONTH);

            	fCalendar.clear();
            	fCalendar.set(Calendar.ERA, era);
            	fCalendar.set(Calendar.YEAR, year);
            	fCalendar.set(Calendar.MONTH, month);
            	fCalendar.set(Calendar.DATE, 1);

            	return fCalendar.getTime();
        	}
    	}

    	private void calculate()
    	{
        	//
        	// As a workaround for JDK 1.1.3 and below, where Calendars and time
        	// zones are a bit goofy, always set my calendar's time zone to UTC.
        	// You would think I would want to do this in the "set" function above,
        	// but if I do that, the program hangs when this class is loaded,
        	// perhaps due to some sort of static initialization ordering problem.
        	// So I do it here instead.
        	//
        	fCalendar.setTimeZone(new SimpleTimeZone(0, "UTC"));

        	Calendar c = (Calendar)fCalendar.clone(); // Temporary copy

        	fStartOfMonth = startOfMonth(fStartOfMonth);

        	// Stash away a few useful constants for this calendar and display
        	minDay = c.getMinimum(Calendar.DAY_OF_WEEK);
        	daysInWeek = c.getMaximum(Calendar.DAY_OF_WEEK) - minDay + 1;

        	firstDayOfWeek = Calendar.getInstance(fDisplayLocale).getFirstDayOfWeek();

        	// Stash away a Date for the start of this month

        	// Find the day of week of the first day in this month
        	c.setTime(fStartOfMonth);
        	firstDayInMonth = c.get(Calendar.DAY_OF_WEEK);

        	// Now find the # of days in the month
        	c.roll(Calendar.DATE, false);
        	daysInMonth = c.get(Calendar.DATE);

        	// Finally, find the end of the month, i.e. the start of the next one
        	c.roll(Calendar.DATE, true);
        	c.add(Calendar.MONTH, 1);
        	c.getTime();        // JDK 1.1.2 bug workaround
        	c.add(Calendar.SECOND, -1);
        	Date endOfMonth = c.getTime();

        	//
        	// Calculate the number of full or partial weeks in this month.
        	// To do this I can just reuse the code that calculates which
        	// calendar cell contains a given date.
        	//
        	numWeeks = dateToCell(daysInMonth).y - dateToCell(1).y + 1;

        	// Remember which holidays fall on which days in this month,
        	// to save the trouble of having to do it later
        	fHolidays.setSize(0);

        	for (int h = 0; h < fAllHolidays.length; h++)
        	{
            	Date d = fStartOfMonth;
            	while ( (d = fAllHolidays[h].firstBetween(d, endOfMonth) ) != null)
            	{
                	c.setTime(d);
                	fHolidays.addElement( new HolidayInfo(c.get(Calendar.DATE),
                                        	fAllHolidays[h],
                                        	fAllHolidays[h].getDisplayName(fDisplayLocale) ));

                	d.setTime( d.getTime() + 1000 );    // "d++"
            	}
        	}
        	dirty = false;
    	}

    	static final int INSET = 2;

    	/*
    	* Convert from the day number within a month (1-based)
    	* to the cell coordinates on the calendar (0-based)
    	*/
    	private void dateToCell(int date, Point pos)
    	{
        	int cell = (date + firstDayInMonth - firstDayOfWeek - minDay);
        	if (firstDayInMonth < firstDayOfWeek) {
            	cell += daysInWeek;
        	}

        	pos.x = cell % daysInWeek;
        	pos.y = cell / daysInWeek;
    	}
    	private Point dateToCell(int date) {
        	Point p = new Point(0,0);
        	dateToCell(date, p);
        	return p;
    	}

    	public void paint(Graphics g) {

        	if (dirty) {
            	calculate();
        	}

        	Point cellPos = new Point(0,0);     // Temporary variable
        	Dimension d = getSize();

        	g.setColor(DemoUtility.bgColor);
        	g.fillRect(0,0,d.width,d.height);

        	// Draw the day names at the top
        	g.setColor(Color.black);
        	g.setFont(DemoUtility.labelFont);
        	FontMetrics fm = g.getFontMetrics();
        	int labelHeight = fm.getHeight() + INSET * 2;

        	int v = fm.getAscent() + INSET;
        	for (int i = 0; i < daysInWeek; i++) {
            	int dayNum = (i + minDay + firstDayOfWeek - 2) % daysInWeek + 1;
            	String dayName = fSymbols.getWeekdays()[dayNum];

            	int h = (int) (d.width * (i + 0.5)) / daysInWeek;
            	h -= fm.stringWidth(dayName) / 2;

            	g.drawString(dayName, h, v);
        	}

        	double cellHeight = (d.height - labelHeight - 1) / numWeeks;
        	double cellWidth = (double)(d.width - 1) / daysInWeek;

        	// Draw a white background in the part of the calendar
        	// that displays this month.
        	// First figure out how much of the first week should be shaded.
        	{
            	g.setColor(Color.white);
            	dateToCell(1, cellPos);
            	int width = (int)(cellPos.x*cellWidth);  // Width of unshaded area

            	g.fillRect((int)(width), labelHeight ,
                    	(int)(d.width - width), (int)cellHeight);

            	// All of the intermediate weeks get shaded completely
            	g.fillRect(0, (int)(labelHeight + cellHeight),
                        	d.width, (int)(cellHeight * (numWeeks - 2)));

            	// Now figure out the last week.
            	dateToCell(daysInMonth, cellPos);
            	width = (int)((cellPos.x+1)*cellWidth);  // Width of shaded area

            	g.fillRect(0, (int)(labelHeight + (numWeeks-1) * cellHeight),
                        	width, (int)(cellHeight));

        	}
        	// Draw the X/Y grid lines
        	g.setColor(Color.black);
        	for (int i = 0; i <= numWeeks; i++) {
            	int y = (int)(labelHeight + i * cellHeight);
            	g.drawLine(0, y, d.width - 1, y);
        	}
        	for (int i = 0; i <= daysInWeek; i++) {
            	int x = (int)(i * cellWidth);
            	g.drawLine(x, labelHeight, x, d.height - 1);
        	}

        	// Now loop through all of the days in the month, figure out where
        	// they go in the grid, and draw the day # for each one
        	Font numberFont = new Font("Helvetica",Font.PLAIN,12);
        	// not used Font holidayFont = DemoUtility.creditFont;

        	Calendar c = (Calendar)fCalendar.clone();
        	c.setTime(fStartOfMonth);

        	for (int i = 1, h = 0; i <= daysInMonth; i++) {
            	g.setFont(numberFont);
            	g.setColor(Color.black);
            	fm = g.getFontMetrics();

            	dateToCell(i, cellPos);
            	int x = (int)((cellPos.x + 1) * cellWidth);
            	int y = (int)(cellPos.y * cellHeight + labelHeight);

            	StringBuffer buffer = new StringBuffer();
            	buffer.append(i);
            	String dayNum = buffer.toString();

            	x = x - INSET - fm.stringWidth(dayNum);
            	y = y + fm.getAscent() + INSET;

            	g.drawString(dayNum, x, y);

            	// See if any of the holidays land on this day....
            	HolidayInfo info = null;
            	int count = 0;

            	// Coordinates of lower-left corner of cell.
            	x = (int)((cellPos.x) * cellWidth);
            	y = (int)((cellPos.y+1) * cellHeight) + labelHeight;

            	while (h < fHolidays.size() &&
                    	(info = (HolidayInfo)fHolidays.elementAt(h)).date <= i)
            	{
                	if (info.date == i) {
                    	// Draw the holiday here.
                    	g.setFont(numberFont);
                    	g.setColor(Color.red);

                    	DemoTextBox box = new DemoTextBox(g, info.name, (int)(cellWidth - INSET));
                    	box.draw(g, x + INSET, y - INSET - box.getHeight());

                    	y -= (box.getHeight() + INSET);
                    	count++;
                	}
                	h++;
            	}
        	}
    	}

    	// Important state variables
    	private Locale              fCalendarLocale;    // Whose calendar
    	private Calendar            fCalendar;          // Calendar for calculations

    	private Locale              fDisplayLocale;     // How to display it
    	private DateFormatSymbols   fSymbols;           // Symbols for drawing

    	private Date                fStartOfMonth;      // 00:00:00 on first day of month

    	// Cached calculations to make drawing faster.
    	private transient int minDay;           // Minimum legal day #
    	private transient int daysInWeek;       // # of days in a week
    	private transient int firstDayOfWeek;   // First day to display in week
    	private transient int numWeeks;         // # full or partial weeks in month
    	private transient int daysInMonth;      // # days in this month
    	private transient int firstDayInMonth;  // Day of week of first day in month

    	private transient Holiday[] fAllHolidays;
    	private transient Vector    fHolidays = new Vector(5,5);

    	private transient boolean dirty = true;
	}

	private static class HolidayInfo {
    	public HolidayInfo(int date, Holiday holiday, String name) {
        	this.date = date;
        	this.holiday = holiday;
        	this.name = name;
    	}

    	public Holiday holiday;
    	public int date;
    	public String name;
	}
}

