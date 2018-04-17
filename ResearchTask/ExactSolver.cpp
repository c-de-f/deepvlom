#include "stdafx.h"
#include <ctime>
#include "Solver.h"

#pragma once

float ExactSolver::solve() {
	//std::cout << p->tasks.size() << '\n';
	bool sch = true;
	for (auto &i = p->tasks.begin(); i != p->tasks.end(); i++) if (i->second.getStatus() != ::DONE) sch = false;
	if ((p->getLowerBound() > solution.second) && (sch)) { cuts2++; return makespan; } // BnB, lowerbound is min links-feasible finish time
	if ((sch) && (p->makespan() > solution.second)) { cuts2++;  return makespan; }
	

	bool actives = false;
	bool passives = false;

	setStatuses();
	//std::cout << "C\n";
	Task * t_res = NULL;
	for (auto &i = p->tasks.begin(); i != p->tasks.end(); i++)
	{
		Task *t = &i->second;
		//if (statuses[t] != TaskStatus::ACTIVE) actives = true;
		if ((t->getStatus() == TaskStatus::ACTIVE) && (t->noResources()))
		{
			t_res = t;
			//std::cout << "Voila\n";
			t->forceCalculating();
			t->setStatus(TaskStatus::DONE);
		}
	}
	setStatuses();
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

	for (auto t : act)
	{
		//std::cout << "Voici\n";
		//t->setBound(solution.second);
#ifdef VALIDATE
		validate1(t->getName());
#endif // VALIDATE
		//std::cout << t->getName() << ' ' << t->getStatus() << '\n';
		tryTask(t);
		//std::cout << p->tasks["32"].start() << ' ' << p->tasks["32"].getStatus() << '\n';
#ifdef VALIDATE
		int a;
		if (a = validate2(t->getName()) > 0) { /*for (auto &k : p->tasks) std::cout << k.first << ' ' << k.second.start() << ' ' << k.second.getStatus() << '\n';*/ std::cout << "validator says " << a << '\n'; exit(0); }
		//std::cout << "Survived with task " << t->getName() << '\n';
#endif // VALIDATE
		

		for (auto &l : p->tasks) if (l.second.getStatus() != TaskStatus::DONE) { /*std::cout << "Resetting " << l.first << '\n';*/  (&l.second)->setStart(0); }

		setStatuses();
	}


	if (!actives)
	{
		if (!passives)
		{
			if (t_res != NULL)
			{
				//std::cout << t_res->getName() << '\n';
				t_res->setStatus(::ACTIVE);
				t_res->setStart(0);
			}
			iterations++;
			if (!(iterations % 10000)) std::cout << iterations << "  " << (clock() - timer) / CLOCKS_PER_SEC << " cuts: " << cuts2 << '\n';

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
		return makespan;
	}
}