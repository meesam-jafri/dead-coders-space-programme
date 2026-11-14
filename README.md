# dead-coders-space-programme

Space Mission Design Game — Final Game Concept
Your 11-step flow is a strong foundation. Now the objective is to turn it into a complete game experience with real engineering decisions, enjoyable gameplay, and NASA-backed educational content.

I recommend naming the current playable version Mission: Lunar Recon.

The player manages a small space agency. They receive a mission to study the Moon, choose their spacecraft and personnel, install equipment, prepare a launch, and operate the spacecraft through difficult situations.

Every decision has consequences. A cheaper spacecraft may have less power, an advanced camera may generate too much data, and forgetting to hire an engineer could make a technical failure harder to resolve.
One important scientific decision
For your first Moon mission, make the default objective lunar orbital reconnaissance, inspired by NASA's Lunar Reconnaissance Orbiter (LRO).

NASA's real LRO mission uses cameras, laser altimetry, radar and other instruments to study lunar terrain, hazards and possible resources. 
NASA Science+1


You can still offer spacecraft with seats and recruit doctors, but distinguish between:
- Robotic spacecraft: No onboard crew, no seats or doctor required. Scientists and engineers operate it from Earth.
- Crewed spacecraft: Requires seats, astronauts, life support, food, oxygen and additional safety requirements. Doctors or flight surgeons support crew health.
For this hackathon, build the complete robotic Moon mission first. Keep crewed gameplay and later planets as expandable features.
1. Your final 11-stage flow, expanded
01

Main Menu

Start Mission, How to Play, Mission Archive, Settings

Gameplay: Animated spacecraft, atmospheric background, responsive buttons


02

Select Mission

Moon unlocked; Mars and Jupiter locked. Display science objective, budget, difficulty and success requirements

Gameplay: Mission cards, planet animations, progress badges and simple explanations


03

Select Spacecraft

Choose chassis with limits for mass, equipment slots, power, crew capacity and destination compatibility

Gameplay: Large spacecraft preview; compare alternatives and see what each can carry


04

Make Your Team

Hire scientists, engineers, communications operators and other specialists within a staffing budget

Gameplay: Character cards with abilities, salaries and event-specific benefits


05

Select Payloads

Attach cameras, sensors, solar panels, batteries, antennas, storage and optional systems

Gameplay: Physical spacecraft assembly, snapping animations, resource bars and information tooltips


06

Select Rocket

Check rocket price, compatible destinations and payload delivery capability

Gameplay: Rocket comparison cards, animated rocket preview and launch-readiness checks


07

Select Orbit

Choose lunar science orbit, observation coverage and simplified fuel requirements

Gameplay: Interactive Moon orbit diagram with animated trajectory previews


08

Select Launch Date

Compare launch windows, wind, lightning risks, readiness and possible delays

Gameplay: Interactive calendar, weather indicators and countdown preparation


09

Launch

Final verification, countdown, liftoff, ascent, stage separation, lunar transfer and orbit insertion

Gameplay: Cinematic animation, engine sounds, screen shake and mission milestones


10

Mission Operations

Manage instruments, battery, science data, communications, failures and specialists

Gameplay: Interactive mission-control gameplay with meaningful decisions and visible consequences


11

Mission Results

Evaluate science delivered, requirements achieved, spacecraft safety, and cost efficiency

Gameplay: Mission grade, animated rewards, NASA learning insights and redesign option



2. The research connection
NASA's Design and Develop Science Missions explains that real missions are built around requirements, design, verification and implementation. It also identifies four segments: launch, space, ground and user. 
NASA


Your 11 stages cover most of that process. The most valuable addition is a Mission Requirements and Engineering Review system that runs across the existing steps, rather than creating another lengthy menu.

For example, the player's lunar mission dossier could specify:
MISSION DOSSIER — LUNAR RECON
Level 01

Illustrative requirements for your fictional mission
- Capture 10 useful lunar observations.
- Successfully return at least 70% of required science data.
- Maintain enough battery power for critical spacecraft functions.
- Enter a compatible lunar orbit.
- Stay within the approved budget.
  
  Mission objective: Help scientists identify promising lunar locations for future exploration.

These objectives become the game's rules. Every spacecraft, payload, rocket and orbit decision can be evaluated against them.
3. Mission Operations — the main gameplay
This is where the player should spend the most time actually playing.

Your attached Anomaly Run concept already has a useful mechanic: unexpected problems force the player to choose between protecting power, data and equipment. We can develop that into a more realistic system where available responses depend on the spacecraft and team selected earlier. Pasted markdown(3).md

Example: Lunar Mission Control
MISSION TIME: DAY 03
Power Warning

Battery

31%





Data Storage

78%






LUNAR ECLIPSE DETECTED

Your solar panels are no longer receiving sunlight. Your battery is draining while the camera continues collecting data.
Choose your response:
Shut down cameraSave electricity, lose observation timeEnter safe modePreserve critical systems, pause scienceContinue observationCollect more data, risk battery depletion
Interactive gameplay example, not a live mission simulation.
Six events worth implementing
Event	What the player must decide
Low battery	Stop experiments or risk losing power
Communications blackout	Store data until Earth contact returns
Camera overheating	Pause science or risk damaging the instrument
Antenna malfunction	Switch to backup or attempt remote recovery
Storage nearly full	Prioritize, compress, transmit or pause observations
Attitude-control fault	Recalibrate pointing or suspend science

For scientific accuracy, an engineer on Earth can help troubleshoot or reconfigure a spacecraft, but cannot physically repair a broken component without suitable in-space servicing.

A particularly strong mechanic is a Moon far-side communication blackout. NASA documents that Earth-based communications are blocked when the Moon obstructs the signal. 
NASA


Imagine how good that could look: the satellite disappears behind the Moon, the radio beam fades, the screen displays SIGNAL LOST, and the player must decide how to manage storage until the connection returns.
4. Make the research part of gameplay
Don't make a separate research chapter that players must read. Attach learning directly to decisions.
Official resource	Research to extract	Feature to build
NASA Mission Development	Objectives, requirements, mission segments	Mission dossier and engineering checks
Mission Design Lab	Engineering trade studies	Compare two spacecraft designs
CubeSat 101	Satellite subsystems and development	Equipment requirements and verification
Lunar Reconnaissance Orbiter	Lunar science instruments	Authentic instrument functions
NASA Launch Guide	Launch windows	Date selection beyond weather
Mission Viz	Trajectory presentation	Animated lunar orbital visualization

NASA's Mission Design Lab specifically uses concept development, feasibility assessment and trade studies. That gives you a real engineering process to base your review mechanics on. 
Goddard Engineering and Technology Directorate


A good payload tooltip would say:

Laser Altimeter: Measures surface elevation using laser pulses. Helps map lunar terrain. Consumes power and adds mass. Inspired by NASA's Lunar Orbiter Laser Altimeter (LOLA). 
NASA Science

5. The project document for Codex
I've put the full plan into a single Markdown document with screen details, gameplay rules, research links, operations, UI, implementation order and Codex instructions.
SPACE_MISSION_MASTER_PLAN.md

241 lines · Full game design + research + technical plan


Download Master Game Plan

You can place it in the project root for Codex to read. It doesn't require changing your current C++ code.
6. What you should work on next
Since you are handling game design and visuals, I would divide your own work into three immediate tasks.

First: Design three screens

Focus on Mission Selection, Spacecraft Builder and Mission Operations. Sketch their layout, button positions and important information before adding detailed artwork.



Second: Research the Moon mission

Study NASA LRO and identify three instruments, their functions and their trade-offs. Create short beginner-friendly descriptions.



Third: Test the game feel

Make sure selecting a component changes the spacecraft visibly. Check button readability, sound feedback, transitions and whether every screen has one obvious next action.