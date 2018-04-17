#include "stdafx.h"
#include <ctime>
#include "Solver.h"

#pragma once

int whatever = 1;

float SprecherSolver::solve() {
	//std::cout << p->tasks.size() << '\n';
	bool sch = true;
	
	for (auto &i = p->tasks.begin(); i != p->tasks.end(); i++) if (i->second.getStatus() != ::DONE) sch = false;
	if ((!continuable) && (sch)) { std::cout << "makesp1: " << makespan << '\n'; return p->makespan(); }
	if ((p->getLowerBound() > solution.second) && (sch)) { std::cout << "makesp2: " << makespan << '\n'; return p->makespan(); } // BnB, lowerbound is min links-feasible finish time
	if ((sch) && (p->makespan() > solution.second)) { std::cout << "makesp3: " << makespan << '\n'; return p->makespan(); }


	bool actives = false;
	bool passives = false;

	setStatuses();
	//std::cout << "C\n";
	Task* t_res = NULL;
	for (auto &i = p->tasks.begin(); i != p->tasks.end(); i++)
	{
		Task *t = &i->second;
		//if (statuses[t] != TaskStatus::ACTIVE) actives = true;
		if ((t->getStatus() == TaskStatus::ACTIVE) && (t->noResources()))
		{
			t_res = t;
			//setMaxStart();
			//if (maxStart + t->getDuration() > solution.second) return makespan;
			t->forceCalculating();

			//if (t->exceeds(solution.second))
			t->setStatus(TaskStatus::DONE);
		}
	}
	setStatuses();
	//for (auto &l = p->tasks.begin(); l != p->tasks.end(); l++) if (l->second.getStatus() != TaskStatus::DONE) l->second.setStart(0);
	std::vector<Task*> act;
	for (auto &i = p->tasks.begin(); i != p->tasks.end(); i++)
	{
		Task *t = &i->second;
		if (t->getStatus() != TaskStatus::ACTIVE)
		{
			if (t->getStatus() == TaskStatus::PASSIVE) passives = true;
			continue;
		}
		actives = true;
		act.push_back(t);
	}

	act = sortedByHeur(act);
	int cnt = 0;
	for (auto t : act)
	{
		continuable = true;
		setMaxStart();
		//if (maxStart + t->getDuration() > solution.second) return makespan;
		t->setBound(solution.second);
#ifdef VALIDATE
		validate1(t->getName());
#endif // VALIDATE
		tryTask(t, maxStart);
	//	if ((((clock() - timer) / CLOCKS_PER_SEC) == whatever)) { std::cout << whatever << '\n'; whatever += 50; }
		/*if (!continuable) {
			continue; }*/
#ifdef VALIDATE
		int a;
		if (a = validate2(t->getName()) > 0) { /*for (auto &k : p->tasks) std::cout << k.first << ' ' << k.second.start() << ' ' << k.second.getStatus() << '\n';*/
			std::cout << "validator3 says " << a << '\n';
			exit(0);
		}
		
#endif // VALIDATE

		
		//for (auto &l = p->tasks.begin(); l != p->tasks.end(); l++) if (l->second.getStatus() != TaskStatus::DONE) l->second.setStart(0);

	/*	setStatuses();
		for (auto &l = p->tasks.begin(); l != p->tasks.end(); l++) if (l->second.getStatus() != TaskStatus::DONE) l->second.setStart(0);*/
	}


	if ((!actives))
	{
		if (!passives)
		{
			if (t_res != NULL)
			{
				//std::cout << "Ici: " <<t_res->getName() << '\n';
				//exit(0);
				t_res->setStatus(::ACTIVE);
				t_res->setStart(0);
				setStatuses();
			}
			iterations++;
			if (!(iterations % 1000)) std::cout << iterations << "  " << (clock() - timer) / CLOCKS_PER_SEC << '\n';

			std::pair<std::map<std::string, Task>, float> m;

			m.first = p->tasks;
			m.second = p->makespan();
			if ((m.second) < solution.second)
			{
				solution.first = m.first;
				solution.second = m.second;
				
				//makespan = std::min(makespan, solution.second);

			}
		}
		return p->makespan();
	}
}